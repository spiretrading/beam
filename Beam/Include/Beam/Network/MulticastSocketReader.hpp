#ifndef BEAM_MULTICAST_SOCKET_READER_HPP
#define BEAM_MULTICAST_SOCKET_READER_HPP
#include "Beam/IO/Reader.hpp"
#include "Beam/Network/MulticastSocket.hpp"

namespace Beam {
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocketChannel;

  /**
   * Implements the Reader interface for a multicast socket.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocketReader {
    public:
      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination);
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size);

    private:
      friend class BasicMulticastSocketChannel<R>;
      std::shared_ptr<BasicMulticastSocket<R>> m_socket;

      explicit BasicMulticastSocketReader(
        std::shared_ptr<BasicMulticastSocket<R>> socket);
      BasicMulticastSocketReader(const BasicMulticastSocketReader&) = delete;
      BasicMulticastSocketReader& operator =(
        const BasicMulticastSocketReader&) = delete;
  };

  /** The on-demand MulticastSocketReader type. */
  using MulticastSocketReader = BasicMulticastSocketReader<UdpSocketReceiver>;

  /** The buffered MulticastSocketReader type. */
  using BufferedMulticastSocketReader =
    BasicMulticastSocketReader<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  bool BasicMulticastSocketReader<R>::poll() const {
    return m_socket->get_receiver().poll();
  }

  template<IsUdpSocketReceiver R>
  template<IsBuffer B>
  std::size_t BasicMulticastSocketReader<R>::read(Out<B> destination) {
    return read(out(destination), std::size_t(-1));
  }

  template<IsUdpSocketReceiver R>
  template<IsBuffer B>
  std::size_t BasicMulticastSocketReader<R>::read(
      Out<B> destination, std::size_t size) {
    return m_socket->get_receiver().receive(out(destination), size);
  }

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketReader<R>::BasicMulticastSocketReader(
    std::shared_ptr<BasicMulticastSocket<R>> socket)
    : m_socket(std::move(socket)) {}
}

#endif
