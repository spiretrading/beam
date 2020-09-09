#ifndef BEAM_UDP_SOCKET_HPP
#define BEAM_UDP_SOCKET_HPP
#include <string>
#include <boost/asio/ip/udp.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/IO/ConnectException.hpp"
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

namespace Beam::Network {

  /** Implements a UDP socket. */
  class UdpSocket {
    public:

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      UdpSocket(const IpAddress& address,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param options The options to apply to this socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      UdpSocket(const IpAddress& address, const UdpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param interface The interface to use.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      UdpSocket(const IpAddress& address, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param interface The interface to use.
       * @param options The options to apply to this socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      UdpSocket(const IpAddress& address, const IpAddress& interface,
        const UdpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      ~UdpSocket();

      /** Returns the IpAddress to send and receive from. */
      const IpAddress& GetAddress() const;

      /** Returns the socket's receiver. */
      UdpSocketReceiver& GetReceiver();

      /** Returns the socket's sender. */
      UdpSocketSender& GetSender();

      void Close();

    private:
      friend class UdpSocketReader;
      IpAddress m_address;
      SocketThreadPool* m_socketThreadPool;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::optional<UdpSocketReceiver> m_receiver;
      boost::optional<UdpSocketSender> m_sender;
      IO::OpenState m_openState;

      UdpSocket(const UdpSocket&) = delete;
      UdpSocket& operator =(const UdpSocket&) = delete;
      void Open(boost::optional<IpAddress> interface,
        const UdpSocketOptions& options);
  };

  inline UdpSocket::UdpSocket(const IpAddress& address,
    Ref<SocketThreadPool> socketThreadPool)
    : UdpSocket(address, UdpSocketOptions(), Ref(socketThreadPool)) {}

  inline UdpSocket::UdpSocket(const IpAddress& address,
      const UdpSocketOptions& options,
      Ref<SocketThreadPool> socketThreadPool)
      : m_address(address),
        m_socketThreadPool(socketThreadPool.Get()),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          m_socketThreadPool->GetService(), m_socketThreadPool->GetService(),
          boost::asio::ip::udp::v4())) {
    Open(boost::none, options);
  }

  inline UdpSocket::UdpSocket(const IpAddress& address,
    const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
    : UdpSocket(address, interface, UdpSocketOptions(),
        Ref(socketThreadPool)) {}

  inline UdpSocket::UdpSocket(const IpAddress& address,
      const IpAddress& interface, const UdpSocketOptions& options,
      Ref<SocketThreadPool> socketThreadPool)
      : m_address(address),
        m_socketThreadPool(socketThreadPool.Get()),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          m_socketThreadPool->GetService(), m_socketThreadPool->GetService(),
          boost::asio::ip::udp::v4())) {
    Open(interface, options);
  }

  inline UdpSocket::~UdpSocket() {
    Close();
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

  inline void UdpSocket::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_socket->Close();
    m_openState.Close();
  }

  inline void UdpSocket::Open(boost::optional<IpAddress> interface,
      const UdpSocketOptions& options) {
    try {
      auto errorCode = boost::system::error_code();
      auto resolver = boost::asio::ip::udp::resolver(*m_socket->m_ioService);
      auto query = boost::asio::ip::udp::resolver::query(m_address.GetHost(),
        std::to_string(m_address.GetPort()));
      auto end = boost::asio::ip::udp::resolver::iterator();
      auto endpointIterator = resolver.resolve(query, errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
      auto isAddressResolved = false;
      while(endpointIterator != end) {
        auto endPoint = boost::asio::ip::udp::endpoint(*endpointIterator);
        auto address = endPoint.address().to_string();
        if(!address.empty()) {
          m_address = IpAddress(address, m_address.GetPort());
          isAddressResolved = true;
          break;
        }
        ++endpointIterator;
      }
      if(!isAddressResolved) {
        BOOST_THROW_EXCEPTION(IO::ConnectException(
          "Unable to resolve IP address."));
      }
      m_socket->m_socket.set_option(
        boost::asio::ip::udp::socket::reuse_address(true), errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
      if(interface) {
        m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
          boost::asio::ip::address_v4::from_string(interface->GetHost()),
          interface->GetPort()), errorCode);
        if(errorCode) {
          BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
            errorCode.message()));
        }
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
