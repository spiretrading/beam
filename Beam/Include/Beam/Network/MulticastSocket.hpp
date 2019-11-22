#ifndef BEAM_MULTICASTSOCKET_HPP
#define BEAM_MULTICASTSOCKET_HPP
#include <optional>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"
#include "Beam/Network/UdpSocketSender.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace Network {

  /*! \class MulticastSocket
      \brief Implements a UDP socket used to join a multicast group.
   */
  class MulticastSocket : private boost::noncopyable {
    public:

      //! Constructs a MulticastSocket.
      /*!
        \param group The multicast group to join.
        \param interface The interface to listen on.
        \param socketThreadPool The thread pool used for the sockets.
      */
      MulticastSocket(const IpAddress& group, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      ~MulticastSocket();

      //! Returns the Settings used by the UdpSocketReceiver.
      const UdpSocketReceiver::Settings& GetReceiverSettings() const;

      //! Sets the Settings used by the UdpSocketReceiver.
      /*!
        \param settings The Settings for the UdpSocketReceiver to use.
      */
      void SetReceiverSettings(const UdpSocketReceiver::Settings& settings);

      //! Returns the multicast group to join.
      const IpAddress& GetGroup() const;

      //! Sets the TTL on the socket.
      /*!
        \param ttl The TTL on the socket.
      */
      void SetTtl(int ttl);

      //! Sets the loop-back option.
      /*!
        \param enable <code>true</code> iff you wish to enable the multicast
               loopback.
      */
      void SetLoopback(bool enable);

      //! Returns the socket's receiver.
      UdpSocketReceiver& GetReceiver();

      //! Returns the socket's sender.
      UdpSocketSender& GetSender();

      void Open();

      void Close();

    private:
      friend class MulticastSocketReader;
      IpAddress m_group;
      IpAddress m_interface;
      SocketThreadPool* m_socketThreadPool;
      UdpSocketReceiver::Settings m_receiverSettings;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      std::optional<UdpSocketReceiver> m_receiver;
      std::optional<UdpSocketSender> m_sender;
      IO::OpenState m_openState;

      void Shutdown();
      void Reset();
  };

  inline MulticastSocket::MulticastSocket(const IpAddress& group,
      const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
      : m_group(group),
        m_interface(interface),
        m_socketThreadPool(socketThreadPool.Get()) {
    Reset();
  }

  inline MulticastSocket::~MulticastSocket() {
    Close();
  }

  inline const UdpSocketReceiver::Settings& MulticastSocket::
      GetReceiverSettings() const {
    return m_receiverSettings;
  }

  inline void MulticastSocket::SetReceiverSettings(
      const UdpSocketReceiver::Settings& settings) {
    m_receiverSettings = settings;
  }

  inline const IpAddress& MulticastSocket::GetGroup() const {
    return m_group;
  }

  inline void MulticastSocket::SetTtl(int ttl) {
    boost::system::error_code errorCode;
    boost::asio::ip::multicast::hops hops(ttl);
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      m_socket->m_socket.set_option(hops, errorCode);
    }
    if(errorCode) {
      BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
        errorCode.message()));
    }
  }

  inline void MulticastSocket::SetLoopback(bool enable) {
    boost::system::error_code errorCode;
    boost::asio::ip::multicast::enable_loopback option(enable);
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      m_socket->m_socket.set_option(option);
    }
    if(errorCode) {
      BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
        errorCode.message()));
    }
  }

  inline UdpSocketReceiver& MulticastSocket::GetReceiver() {
    return *m_receiver;
  }

  inline UdpSocketSender& MulticastSocket::GetSender() {
    return *m_sender;
  }

  inline void MulticastSocket::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      boost::system::error_code errorCode;
      auto outboundInterface = boost::asio::ip::address_v4::from_string(
        m_interface.GetHost(), errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
      errorCode.clear();
      m_socket->m_socket.set_option(
        boost::asio::ip::multicast::outbound_interface(outboundInterface),
        errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
      m_socket->m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(
        true), errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
#ifdef _WIN32
      m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
        boost::asio::ip::udp::v4(), m_group.GetPort()), errorCode);
#else
      m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
        boost::asio::ip::address::from_string(m_group.GetHost()),
        m_group.GetPort()), errorCode);
#endif
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
      boost::asio::ip::multicast::join_group joinGroup(
        boost::asio::ip::address_v4::from_string(m_group.GetHost()),
        boost::asio::ip::address_v4::from_string(m_interface.GetHost()));
      m_socket->m_socket.set_option(joinGroup, errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    try {
      m_receiver->Open(m_receiverSettings);
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_socket->m_isOpen = true;
    m_openState.SetOpen();
  }

  inline void MulticastSocket::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MulticastSocket::Shutdown() {
    m_socket->Close();
    Reset();
    m_openState.SetClosed();
  }

  inline void MulticastSocket::Reset() {
    m_socket.reset();
    m_receiver = std::nullopt;
    m_sender = std::nullopt;
    m_socket = std::make_shared<Details::UdpSocketEntry>(
      m_socketThreadPool->GetService(), m_socketThreadPool->GetService(),
      boost::asio::ip::udp::v4());
    m_receiver.emplace(m_socket);
    m_sender.emplace(m_socket);
  }
}
}

#endif
