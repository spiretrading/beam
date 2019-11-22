#ifndef BEAM_UDPSOCKET_HPP
#define BEAM_UDPSOCKET_HPP
#include <optional>
#include <boost/asio/ip/udp.hpp>
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

  /*! \class UdpSocket
      \brief Implements a UDP socket.
   */
  class UdpSocket : private boost::noncopyable {
    public:

      //! Constructs a UdpSocket.
      /*!
        \param address The address to connect to.
        \param socketThreadPool The thread pool used for the sockets.
      */
      UdpSocket(const IpAddress& address,
        Ref<SocketThreadPool> socketThreadPool);

      //! Constructs a UdpSocket.
      /*!
        \param address The address to connect to.
        \param interface The interface to use.
        \param socketThreadPool The thread pool used for the sockets.
      */
      UdpSocket(const IpAddress& address, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      ~UdpSocket();

      //! Returns the Settings used by the UdpSocketReceiver.
      const UdpSocketReceiver::Settings& GetReceiverSettings() const;

      //! Sets the Settings used by the UdpSocketReceiver.
      /*!
        \param settings The Settings for the UdpSocketReceiver to use.
      */
      void SetReceiverSettings(const UdpSocketReceiver::Settings& settings);

      //! Returns the address.
      const IpAddress& GetAddress() const;

      //! Returns the socket's receiver.
      UdpSocketReceiver& GetReceiver();

      //! Returns the socket's sender.
      UdpSocketSender& GetSender();

      void Open();

      void Close();

    private:
      friend class UdpSocketReader;
      IpAddress m_address;
      std::optional<IpAddress> m_interface;
      SocketThreadPool* m_socketThreadPool;
      UdpSocketReceiver::Settings m_receiverSettings;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      std::optional<UdpSocketReceiver> m_receiver;
      std::optional<UdpSocketSender> m_sender;
      IO::OpenState m_openState;

      void Shutdown();
      void Reset();
  };

  inline UdpSocket::UdpSocket(const IpAddress& address,
      Ref<SocketThreadPool> socketThreadPool)
      : m_address(address),
        m_socketThreadPool(socketThreadPool.Get()) {
    Reset();
  }

  inline UdpSocket::UdpSocket(const IpAddress& address,
      const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
      : m_address(address),
        m_interface(interface),
        m_socketThreadPool(socketThreadPool.Get()) {
    Reset();
  }

  inline UdpSocket::~UdpSocket() {
    Close();
  }

  inline const UdpSocketReceiver::Settings& UdpSocket::
      GetReceiverSettings() const {
    return m_receiverSettings;
  }

  inline void UdpSocket::SetReceiverSettings(
      const UdpSocketReceiver::Settings& settings) {
    m_receiverSettings = settings;
  }

  inline const IpAddress& UdpSocket::GetAddress() const {
    return m_address;
  }

  inline UdpSocketReceiver& UdpSocket::GetReceiver() {
    return *m_receiver;
  }

  inline UdpSocketSender& UdpSocket::GetSender() {
    return *m_sender;
  }

  inline void UdpSocket::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    boost::system::error_code errorCode;
    boost::asio::ip::udp::resolver resolver{*m_socket->m_ioService};
    boost::asio::ip::udp::resolver::query query{m_address.GetHost(),
      ToString(m_address.GetPort())};
    boost::asio::ip::udp::resolver::iterator end;
    auto endpointIterator = resolver.resolve(query, errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    auto isAddressResolved = false;
    while(endpointIterator != end) {
      boost::asio::ip::udp::endpoint endPoint = *endpointIterator;
      auto address = endPoint.address().to_string();
      if(!address.empty()) {
        m_address = IpAddress{address, m_address.GetPort()};
        isAddressResolved = true;
        break;
      }
      ++endpointIterator;
    }
    if(!isAddressResolved) {
      m_openState.SetOpenFailure(IO::ConnectException{
        "Unable to resolve IP address."});
      Shutdown();
    }
    m_socket->m_socket.set_option(
      boost::asio::ip::udp::socket::reuse_address(true), errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    if(m_interface.has_value()) {
      m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
        boost::asio::ip::address_v4::from_string(m_interface->GetHost()),
        m_interface->GetPort()), errorCode);
      if(errorCode) {
        m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
        Shutdown();
      }
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

  inline void UdpSocket::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void UdpSocket::Shutdown() {
    m_socket->Close();
    Reset();
    m_openState.SetClosed();
  }

  inline void UdpSocket::Reset() {
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
