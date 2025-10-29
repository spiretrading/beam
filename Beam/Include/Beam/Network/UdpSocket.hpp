#ifndef BEAM_UDP_SOCKET_HPP
#define BEAM_UDP_SOCKET_HPP
#include <string>
#include <boost/asio/ip/udp.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"
#include "Beam/Network/UdpSocketSender.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /** Implements a UDP socket. */
  class UdpSocket {
    public:

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       */
      explicit UdpSocket(const IpAddress& address);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param options The options to apply to this socket.
       */
      UdpSocket(const IpAddress& address, const UdpSocketOptions& options);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param interface The interface to use.
       */
      UdpSocket(const IpAddress& address, const IpAddress& interface);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param interface The interface to use.
       * @param options The options to apply to this socket.
       */
      UdpSocket(const IpAddress& address, const IpAddress& interface,
        const UdpSocketOptions& options);

      ~UdpSocket();

      /** Returns the IpAddress to send and receive from. */
      const IpAddress& get_address() const;

      /** Returns the socket's receiver. */
      UdpSocketReceiver& get_receiver();

      /** Returns the socket's sender. */
      UdpSocketSender& get_sender();

      void close();

    private:
      friend class UdpSocketReader;
      IpAddress m_address;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::optional<UdpSocketReceiver> m_receiver;
      boost::optional<UdpSocketSender> m_sender;
      OpenState m_open_state;

      UdpSocket(const UdpSocket&) = delete;
      UdpSocket& operator =(const UdpSocket&) = delete;
      void open(
        boost::optional<IpAddress> interface, const UdpSocketOptions& options);
  };

  inline UdpSocket::UdpSocket(const IpAddress& address)
    : UdpSocket(address, UdpSocketOptions()) {}

  inline UdpSocket::UdpSocket(
      const IpAddress& address, const UdpSocketOptions& options)
      : m_address(address),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          ServiceThreadPool::get().get_context(),
          ServiceThreadPool::get().get_context(), boost::asio::ip::udp::v4())) {
    open(boost::none, options);
  }

  inline UdpSocket::UdpSocket(
    const IpAddress& address, const IpAddress& interface)
    : UdpSocket(address, interface, UdpSocketOptions()) {}

  inline UdpSocket::UdpSocket(const IpAddress& address,
      const IpAddress& interface, const UdpSocketOptions& options)
      : m_address(address),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          ServiceThreadPool::get().get_context(),
          ServiceThreadPool::get().get_context(),
          boost::asio::ip::udp::v4())) {
    open(interface, options);
  }

  inline UdpSocket::~UdpSocket() {
    close();
  }

  inline const IpAddress& UdpSocket::get_address() const {
    return m_address;
  }

  inline UdpSocketReceiver& UdpSocket::get_receiver() {
    return *m_receiver;
  }

  inline UdpSocketSender& UdpSocket::get_sender() {
    return *m_sender;
  }

  inline void UdpSocket::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_socket->close();
    m_open_state.close();
  }

  inline void UdpSocket::open(
      boost::optional<IpAddress> interface, const UdpSocketOptions& options) {
    try {
      auto error_code = boost::system::error_code();
      auto resolver = boost::asio::ip::udp::resolver(*m_socket->m_io_context);
      auto ends = resolver.resolve(
        m_address.get_host(), std::to_string(m_address.get_port()), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      auto is_address_resolved = false;
      for(auto& end : ends) {
        auto address = end.host_name();
        if(!address.empty()) {
          auto ip_addresses = resolver.resolve(boost::asio::ip::udp::v4(),
            address, std::to_string(m_address.get_port()));
          if(ip_addresses.empty()) {
            m_address = IpAddress(address, m_address.get_port());
          } else {
            m_address = IpAddress(
              ip_addresses.begin()->endpoint().address().to_string(),
              m_address.get_port());
          }
          is_address_resolved = true;
          break;
        }
      }
      if(!is_address_resolved) {
        boost::throw_with_location(
          ConnectException("Unable to resolve IP address."));
      }
      m_socket->m_socket.set_option(
        boost::asio::ip::udp::socket::reuse_address(true), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      if(interface) {
        m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
          boost::asio::ip::make_address_v4(interface->get_host()),
          interface->get_port()), error_code);
        if(error_code) {
          boost::throw_with_location(
            SocketException(error_code.value(), error_code.message()));
        }
      }
      m_receiver.emplace(options, m_socket);
      m_sender.emplace(options, m_socket);
    } catch(const ConnectException&) {
      close();
      throw;
    } catch(const std::exception&) {
      close();
      std::throw_with_nested(ConnectException("Unable to open socket."));
    }
    m_socket->m_is_open = true;
  }
}

#endif
