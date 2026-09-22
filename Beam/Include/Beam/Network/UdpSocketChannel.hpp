#ifndef BEAM_UDP_SOCKET_CHANNEL_HPP
#define BEAM_UDP_SOCKET_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketConnection.hpp"
#include "Beam/Network/UdpSocketReader.hpp"
#include "Beam/Network/UdpSocketWriter.hpp"

namespace Beam {

  /**
   * Implements the Channel interface using a UDP socket.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicUdpSocketChannel {
    public:
      using Identifier = SocketIdentifier;
      using Connection = BasicUdpSocketConnection<R>;
      using Reader = BasicUdpSocketReader<R>;
      using Writer = BasicUdpSocketWriter<R>;

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       */
      explicit BasicUdpSocketChannel(const IpAddress& address);

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       * @param options The options to apply to the socket.
       */
      BasicUdpSocketChannel(
        const IpAddress& address, const UdpSocketOptions& options);

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       * @param interface The interface to use.
       */
      BasicUdpSocketChannel(
        const IpAddress& address, const IpAddress& interface);

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       * @param interface The interface to use.
       * @param options The options to apply to the socket.
       */
      BasicUdpSocketChannel(
        const IpAddress& address, const IpAddress& interface,
        const UdpSocketOptions& options);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      Identifier m_identifier;
      std::shared_ptr<BasicUdpSocket<R>> m_socket;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      BasicUdpSocketChannel(const BasicUdpSocketChannel&) = delete;
      BasicUdpSocketChannel& operator =(const BasicUdpSocketChannel&) = delete;
  };

  /** The on-demand UdpSocketChannel type. */
  using UdpSocketChannel = BasicUdpSocketChannel<UdpSocketReceiver>;

  /** The buffered UdpSocketChannel type. */
  using BufferedUdpSocketChannel =
    BasicUdpSocketChannel<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  BasicUdpSocketChannel<R>::BasicUdpSocketChannel(const IpAddress& address)
    : BasicUdpSocketChannel<R>(address, UdpSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicUdpSocketChannel<R>::BasicUdpSocketChannel(
    const IpAddress& address, const UdpSocketOptions& options)
    : BasicUdpSocketChannel<R>(address, IpAddress("0.0.0.0", 0), options) {}

  template<IsUdpSocketReceiver R>
  BasicUdpSocketChannel<R>::BasicUdpSocketChannel(
    const IpAddress& address, const IpAddress& interface)
    : BasicUdpSocketChannel<R>(address, interface, UdpSocketOptions()) {}

  template<IsUdpSocketReceiver R>
  BasicUdpSocketChannel<R>::BasicUdpSocketChannel(const IpAddress& address,
    const IpAddress& interface, const UdpSocketOptions& options)
    : m_identifier(address),
      m_socket(
        std::make_shared<BasicUdpSocket<R>>(address, interface, options)),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  template<IsUdpSocketReceiver R>
  const typename BasicUdpSocketChannel<R>::Identifier&
      BasicUdpSocketChannel<R>::get_identifier() const {
    return m_identifier;
  }

  template<IsUdpSocketReceiver R>
  typename BasicUdpSocketChannel<R>::Connection&
      BasicUdpSocketChannel<R>::get_connection() {
    return m_connection;
  }

  template<IsUdpSocketReceiver R>
  typename BasicUdpSocketChannel<R>::Reader&
      BasicUdpSocketChannel<R>::get_reader() {
    return m_reader;
  }

  template<IsUdpSocketReceiver R>
  typename BasicUdpSocketChannel<R>::Writer&
      BasicUdpSocketChannel<R>::get_writer() {
    return m_writer;
  }
}

#endif
