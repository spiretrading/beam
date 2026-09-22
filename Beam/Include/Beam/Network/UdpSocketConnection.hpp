#ifndef BEAM_UDP_SOCKET_CONNECTION_HPP
#define BEAM_UDP_SOCKET_CONNECTION_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/UdpSocket.hpp"

namespace Beam {
  template<IsUdpSocketReceiver R>
  class BasicUdpSocketChannel;

  /**
   * Provides a Connection interface for a UDP socket.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicUdpSocketConnection {
    public:
      ~BasicUdpSocketConnection();

      void close();

    private:
      friend class BasicUdpSocketChannel<R>;
      std::shared_ptr<BasicUdpSocket<R>> m_socket;

      explicit BasicUdpSocketConnection(
        std::shared_ptr<BasicUdpSocket<R>> socket);
      BasicUdpSocketConnection(const BasicUdpSocketConnection&) = delete;
      BasicUdpSocketConnection& operator =(
        const BasicUdpSocketConnection&) = delete;
  };

  /** The on-demand UdpSocketConnection type. */
  using UdpSocketConnection = BasicUdpSocketConnection<UdpSocketReceiver>;

  /** The buffered UdpSocketConnection type. */
  using BufferedUdpSocketConnection =
    BasicUdpSocketConnection<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  BasicUdpSocketConnection<R>::~BasicUdpSocketConnection() {
    close();
  }

  template<IsUdpSocketReceiver R>
  void BasicUdpSocketConnection<R>::close() {
    m_socket->close();
  }

  template<IsUdpSocketReceiver R>
  BasicUdpSocketConnection<R>::BasicUdpSocketConnection(
    std::shared_ptr<BasicUdpSocket<R>> socket)
    : m_socket(std::move(socket)) {}
}

#endif
