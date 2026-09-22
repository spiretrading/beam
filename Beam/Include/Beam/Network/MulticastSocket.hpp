#ifndef BEAM_MULTICAST_SOCKET_HPP
#define BEAM_MULTICAST_SOCKET_HPP
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/BufferedUdpSocketReceiver.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/MulticastSocketOptions.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/UdpSocketSender.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /**
   * Implements a UDP socket used to join a multicast group.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocket {
    public:

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       */
      explicit BasicMulticastSocket(const IpAddress& group);

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       */
      BasicMulticastSocket(
        const IpAddress& group, const MulticastSocketOptions& options);

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       * @param interface The interface to listen on.
       */
      BasicMulticastSocket(const IpAddress& group, const IpAddress& interface);

      /**
       * Constructs a MulticastSocket.
       * @param group The multicast group to join.
       * @param interface The interface to listen on.
       */
      BasicMulticastSocket(const IpAddress& group, const IpAddress& interface,
        const MulticastSocketOptions& options);

      ~BasicMulticastSocket();

      /** Returns the multicast group to join. */
      const IpAddress& get_group() const;

      /** Returns the socket's receiver. */
      R& get_receiver();

      /** Returns the socket's sender. */
      UdpSocketSender& get_sender();

      void close();

    private:
      IpAddress m_group;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::optional<R> m_receiver;
      boost::optional<UdpSocketSender> m_sender;
      OpenState m_open_state;

      BasicMulticastSocket(const BasicMulticastSocket&) = delete;
      BasicMulticastSocket& operator =(const BasicMulticastSocket&) = delete;
      void open(
        const IpAddress& interface, const MulticastSocketOptions& options);
  };

  /** The on-demand MulticastSocket type. */
  using MulticastSocket = BasicMulticastSocket<UdpSocketReceiver>;

  /** The buffered MulticastSocket type. */
  using BufferedMulticastSocket =
    BasicMulticastSocket<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  BasicMulticastSocket<R>::BasicMulticastSocket(const IpAddress& group)
    : BasicMulticastSocket<R>(group, MulticastSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicMulticastSocket<R>::BasicMulticastSocket(
    const IpAddress& group, const MulticastSocketOptions& options)
    : BasicMulticastSocket<R>(group, IpAddress("0.0.0.0", 0), options) {}

  template<IsUdpSocketReceiver R>
  BasicMulticastSocket<R>::BasicMulticastSocket(
    const IpAddress& group, const IpAddress& interface)
    : BasicMulticastSocket<R>(group, interface, MulticastSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicMulticastSocket<R>::BasicMulticastSocket(const IpAddress& group,
      const IpAddress& interface, const MulticastSocketOptions& options)
      : m_group(group),
        m_socket(std::make_shared<Details::UdpSocketEntry>(
          ServiceThreadPool::get().get_context(),
          ServiceThreadPool::get().get_context(), boost::asio::ip::udp::v4())) {
    open(interface, options);
  }

  template<IsUdpSocketReceiver R>
  BasicMulticastSocket<R>::~BasicMulticastSocket() {
    close();
  }

  template<IsUdpSocketReceiver R>
  const IpAddress& BasicMulticastSocket<R>::get_group() const {
    return m_group;
  }

  template<IsUdpSocketReceiver R>
  R& BasicMulticastSocket<R>::get_receiver() {
    return *m_receiver;
  }

  template<IsUdpSocketReceiver R>
  UdpSocketSender& BasicMulticastSocket<R>::get_sender() {
    return *m_sender;
  }

  template<IsUdpSocketReceiver R>
  void BasicMulticastSocket<R>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_socket->close();
    m_open_state.close();
  }

  template<IsUdpSocketReceiver R>
  void BasicMulticastSocket<R>::open(
      const IpAddress& interface, const MulticastSocketOptions& options) {
    try {
      auto error_code = boost::system::error_code();
      auto outbound_interface =
        boost::asio::ip::make_address_v4(interface.get_host(), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      m_socket->m_socket.set_option(
        boost::asio::ip::multicast::outbound_interface(outbound_interface),
        error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      m_socket->m_socket.set_option(
        boost::asio::ip::udp::socket::reuse_address(true), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
#ifdef _WIN32
      m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
        boost::asio::ip::udp::v4(), m_group.get_port()), error_code);
#else
      m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
        boost::asio::ip::make_address_v4(m_group.get_host()),
        m_group.get_port()), error_code);
#endif
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      auto join_group = boost::asio::ip::multicast::join_group(
        boost::asio::ip::make_address_v4(m_group.get_host()),
        boost::asio::ip::make_address_v4(interface.get_host()));
      m_socket->m_socket.set_option(join_group, error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      if(options.m_ttl >= 0) {
        m_socket->m_socket.set_option(
          boost::asio::ip::multicast::hops(options.m_ttl), error_code);
        if(error_code) {
          boost::throw_with_location(
            SocketException(error_code.value(), error_code.message()));
        }
      }
      m_socket->m_socket.set_option(boost::asio::ip::multicast::enable_loopback(
        options.m_enable_loopback), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
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
