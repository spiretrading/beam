#ifndef BEAM_MULTICAST_SOCKET_HPP
#define BEAM_MULTICAST_SOCKET_HPP
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/MulticastSocketOptions.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"
#include "Beam/Network/UdpSocketSender.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam::Network {

  /** Implements a UDP socket used to join a multicast group. */
  class MulticastSocket {
    public:

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocket(const IpAddress& group,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocket(const IpAddress& group,
        const MulticastSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       * @param interface The interface to listen on.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocket(const IpAddress& group, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       * @param interface The interface to listen on.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocket(const IpAddress& group, const IpAddress& interface,
        const MulticastSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      ~MulticastSocket();

      /** Returns the multicast group to join. */
      const IpAddress& GetGroup() const;

      /** Returns the socket's receiver. */
      UdpSocketReceiver& GetReceiver();

      /** Returns the socket's sender. */
      UdpSocketSender& GetSender();

      void Close();

    private:
      friend class MulticastSocketReader;
      IpAddress m_group;
      SocketThreadPool* m_socketThreadPool;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::optional<UdpSocketReceiver> m_receiver;
      boost::optional<UdpSocketSender> m_sender;
      IO::OpenState m_openState;

      MulticastSocket(const MulticastSocket&) = delete;
      MulticastSocket& operator =(const MulticastSocket&) = delete;
      void Open(const IpAddress& interface,
        const MulticastSocketOptions& options);
  };

  inline MulticastSocket::MulticastSocket(const IpAddress& group,
    Ref<SocketThreadPool> socketThreadPool)
    : MulticastSocket(group, MulticastSocketOptions(), Ref(socketThreadPool)) {}

  inline MulticastSocket::MulticastSocket(const IpAddress& group,
    const MulticastSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : MulticastSocket(group, IpAddress("0.0.0.0", 0), options,
        Ref(socketThreadPool)) {}

  inline MulticastSocket::MulticastSocket(const IpAddress& group,
    const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
    : MulticastSocket(group, interface, MulticastSocketOptions(),
        Ref(socketThreadPool)) {}

  inline MulticastSocket::MulticastSocket(const IpAddress& group,
      const IpAddress& interface, const MulticastSocketOptions& options,
      Ref<SocketThreadPool> socketThreadPool)
      : m_group(group),
        m_socketThreadPool(socketThreadPool.Get()),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          m_socketThreadPool->GetService(), m_socketThreadPool->GetService(),
          boost::asio::ip::udp::v4())) {
    Open(interface, options);
  }

  inline MulticastSocket::~MulticastSocket() {
    Close();
  }

  inline const IpAddress& MulticastSocket::GetGroup() const {
    return m_group;
  }

  inline UdpSocketReceiver& MulticastSocket::GetReceiver() {
    return *m_receiver;
  }

  inline UdpSocketSender& MulticastSocket::GetSender() {
    return *m_sender;
  }

  inline void MulticastSocket::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_socket->Close();
    m_openState.Close();
  }

  inline void MulticastSocket::Open(const IpAddress& interface,
      const MulticastSocketOptions& options) {
    try {
      auto errorCode = boost::system::error_code();
      auto outboundInterface = boost::asio::ip::address_v4::from_string(
        interface.GetHost(), errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
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
      auto joinGroup = boost::asio::ip::multicast::join_group(
        boost::asio::ip::address_v4::from_string(m_group.GetHost()),
        boost::asio::ip::address_v4::from_string(interface.GetHost()));
      m_socket->m_socket.set_option(joinGroup, errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
      if(options.m_ttl >= 0) {
        m_socket->m_socket.set_option(boost::asio::ip::multicast::hops(
          options.m_ttl), errorCode);
        if(errorCode) {
          BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
            errorCode.message()));
        }
      }
      m_socket->m_socket.set_option(boost::asio::ip::multicast::enable_loopback(
        options.m_enableLoopback), errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
      m_receiver.emplace(options, m_socket);
      m_sender.emplace(options, m_socket);
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
    m_socket->m_isOpen = true;
  }
}

#endif
