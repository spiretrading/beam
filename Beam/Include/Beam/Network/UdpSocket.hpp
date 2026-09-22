#ifndef BEAM_UDP_SOCKET_HPP
#define BEAM_UDP_SOCKET_HPP
#include <string>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/BufferedUdpSocketReceiver.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/UdpSocketSender.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /**
   * Implements a UDP socket.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicUdpSocket {
    public:

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       */
      explicit BasicUdpSocket(const IpAddress& address);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param options The options to apply to this socket.
       */
      BasicUdpSocket(const IpAddress& address, const UdpSocketOptions& options);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param interface The interface to use.
       */
      BasicUdpSocket(const IpAddress& address, const IpAddress& interface);

      /**
       * Constructs a UdpSocket.
       * @param address The address to send to.
       * @param interface The interface to use.
       * @param options The options to apply to this socket.
       */
      BasicUdpSocket(const IpAddress& address, const IpAddress& interface,
        const UdpSocketOptions& options);

      ~BasicUdpSocket();

      /** Returns the IpAddress to send and receive from. */
      const IpAddress& get_address() const;

      /** Returns the socket's receiver. */
      R& get_receiver();

      /** Returns the socket's sender. */
      UdpSocketSender& get_sender();

      void close();

    private:
      IpAddress m_address;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::optional<R> m_receiver;
      boost::optional<UdpSocketSender> m_sender;
      OpenState m_open_state;

      BasicUdpSocket(const BasicUdpSocket&) = delete;
      BasicUdpSocket& operator =(const BasicUdpSocket&) = delete;
      void open(
        boost::optional<IpAddress> interface, const UdpSocketOptions& options);
  };

  /** The on-demand UdpSocket type. */
  using UdpSocket = BasicUdpSocket<UdpSocketReceiver>;

  /** The buffered UdpSocket type. */
  using BufferedUdpSocket = BasicUdpSocket<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  BasicUdpSocket<R>::BasicUdpSocket(const IpAddress& address)
    : BasicUdpSocket<R>(address, UdpSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicUdpSocket<R>::BasicUdpSocket(
      const IpAddress& address, const UdpSocketOptions& options)
      : m_address(address),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          ServiceThreadPool::get().get_context(),
          ServiceThreadPool::get().get_context(), boost::asio::ip::udp::v4())) {
    open(boost::none, options);
  }

  template<IsUdpSocketReceiver R>
  BasicUdpSocket<R>::BasicUdpSocket(
    const IpAddress& address, const IpAddress& interface)
    : BasicUdpSocket<R>(address, interface, UdpSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicUdpSocket<R>::BasicUdpSocket(const IpAddress& address,
      const IpAddress& interface, const UdpSocketOptions& options)
      : m_address(address),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          ServiceThreadPool::get().get_context(),
          ServiceThreadPool::get().get_context(),
          boost::asio::ip::udp::v4())) {
    open(interface, options);
  }

  template<IsUdpSocketReceiver R>
  BasicUdpSocket<R>::~BasicUdpSocket() {
    close();
  }

  template<IsUdpSocketReceiver R>
  const IpAddress& BasicUdpSocket<R>::get_address() const {
    return m_address;
  }

  template<IsUdpSocketReceiver R>
  R& BasicUdpSocket<R>::get_receiver() {
    return *m_receiver;
  }

  template<IsUdpSocketReceiver R>
  UdpSocketSender& BasicUdpSocket<R>::get_sender() {
    return *m_sender;
  }

  template<IsUdpSocketReceiver R>
  void BasicUdpSocket<R>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_socket->close();
    m_open_state.close();
  }

  template<IsUdpSocketReceiver R>
  void BasicUdpSocket<R>::open(
      boost::optional<IpAddress> interface, const UdpSocketOptions& options) {
    try {
      auto error_code = boost::system::error_code();
      auto resolver = boost::asio::ip::udp::resolver(*m_socket->m_io_context);
      auto ends = resolver.resolve(boost::asio::ip::udp::v4(),
        m_address.get_host(), std::to_string(m_address.get_port()), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      if(ends.empty()) {
        boost::throw_with_location(
          ConnectException("Unable to resolve IP address."));
      }
      m_address = IpAddress(
        ends.begin()->endpoint().address().to_string(), m_address.get_port());
      m_socket->m_socket.set_option(
        boost::asio::ip::udp::socket::reuse_address(true), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      if(options.m_ttl >= 0) {
        m_socket->m_socket.set_option(
          boost::asio::ip::unicast::hops(options.m_ttl), error_code);
        if(error_code) {
          boost::throw_with_location(
            SocketException(error_code.value(), error_code.message()));
        }
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
      throw_nested_with_location(ConnectException("Unable to open socket."));
    }
    m_socket->m_is_open = true;
  }
}

#endif
