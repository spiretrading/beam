#ifndef BEAM_MULTICAST_SOCKET_CHANNEL_HPP
#define BEAM_MULTICAST_SOCKET_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/MulticastSocketConnection.hpp"
#include "Beam/Network/MulticastSocketReader.hpp"
#include "Beam/Network/MulticastSocketWriter.hpp"
#include "Beam/Network/SocketIdentifier.hpp"

namespace Beam {

  /**
   * Implements the Channel interface using a multicast socket.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocketChannel {
    public:
      using Identifier = SocketIdentifier;
      using Connection = BasicMulticastSocketConnection<R>;
      using Reader = BasicMulticastSocketReader<R>;
      using Writer = BasicMulticastSocketWriter<R>;

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       */
      explicit BasicMulticastSocketChannel(const IpAddress& group);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param options The options to apply to the socket.
       */
      BasicMulticastSocketChannel(
        const IpAddress& group, const MulticastSocketOptions& options);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param interface The interface to use.
       */
      BasicMulticastSocketChannel(
        const IpAddress& group, const IpAddress& interface);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param interface The interface to use.
       * @param options The options to apply to the socket.
       */
      BasicMulticastSocketChannel(
        const IpAddress& group, const IpAddress& interface,
        const MulticastSocketOptions& options);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      Identifier m_identifier;
      std::shared_ptr<BasicMulticastSocket<R>> m_socket;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      BasicMulticastSocketChannel(const BasicMulticastSocketChannel&) = delete;
      BasicMulticastSocketChannel& operator =(
        const BasicMulticastSocketChannel&) = delete;
  };

  /** The on-demand MulticastSocketChannel type. */
  using MulticastSocketChannel = BasicMulticastSocketChannel<UdpSocketReceiver>;

  /** The buffered MulticastSocketChannel type. */
  using BufferedMulticastSocketChannel =
    BasicMulticastSocketChannel<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketChannel<R>::BasicMulticastSocketChannel(
    const IpAddress& group)
    : BasicMulticastSocketChannel<R>(group, MulticastSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketChannel<R>::BasicMulticastSocketChannel(
    const IpAddress& group, const MulticastSocketOptions& options)
    : m_identifier(group),
      m_socket(std::make_shared<BasicMulticastSocket<R>>(group, options)),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket, group) {}

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketChannel<R>::BasicMulticastSocketChannel(
    const IpAddress& group, const IpAddress& interface)
    : BasicMulticastSocketChannel<R>(
        group, interface, MulticastSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketChannel<R>::BasicMulticastSocketChannel(
    const IpAddress& group, const IpAddress& interface,
    const MulticastSocketOptions& options)
    : m_identifier(group),
      m_socket(
        std::make_shared<BasicMulticastSocket<R>>(group, interface, options)),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket, group) {}

  template<IsUdpSocketReceiver R>
  const typename BasicMulticastSocketChannel<R>::Identifier&
      BasicMulticastSocketChannel<R>::get_identifier() const {
    return m_identifier;
  }

  template<IsUdpSocketReceiver R>
  typename BasicMulticastSocketChannel<R>::Connection&
      BasicMulticastSocketChannel<R>::get_connection() {
    return m_connection;
  }

  template<IsUdpSocketReceiver R>
  typename BasicMulticastSocketChannel<R>::Reader&
      BasicMulticastSocketChannel<R>::get_reader() {
    return m_reader;
  }

  template<IsUdpSocketReceiver R>
  typename BasicMulticastSocketChannel<R>::Writer&
      BasicMulticastSocketChannel<R>::get_writer() {
    return m_writer;
  }
}

#endif
