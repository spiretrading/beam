#ifndef BEAM_MULTICAST_SOCKET_CONNECTION_HPP
#define BEAM_MULTICAST_SOCKET_CONNECTION_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/MulticastSocket.hpp"

namespace Beam {
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocketChannel;

  /**
   * Provides a Connection interface for a multicast socket.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocketConnection {
    public:
      ~BasicMulticastSocketConnection();

      void close();

    private:
      friend class BasicMulticastSocketChannel<R>;
      std::shared_ptr<BasicMulticastSocket<R>> m_socket;

      explicit BasicMulticastSocketConnection(
        std::shared_ptr<BasicMulticastSocket<R>> socket);
      BasicMulticastSocketConnection(
        const BasicMulticastSocketConnection&) = delete;
      BasicMulticastSocketConnection& operator =(
        const BasicMulticastSocketConnection&) = delete;
  };

  /** The on-demand MulticastSocketConnection type. */
  using MulticastSocketConnection =
    BasicMulticastSocketConnection<UdpSocketReceiver>;

  /** The buffered MulticastSocketConnection type. */
  using BufferedMulticastSocketConnection =
    BasicMulticastSocketConnection<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketConnection<R>::~BasicMulticastSocketConnection() {
    close();
  }

  template<IsUdpSocketReceiver R>
  void BasicMulticastSocketConnection<R>::close() {
    m_socket->close();
  }

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketConnection<R>::BasicMulticastSocketConnection(
    std::shared_ptr<BasicMulticastSocket<R>> socket)
    : m_socket(std::move(socket)) {}
}

#endif
