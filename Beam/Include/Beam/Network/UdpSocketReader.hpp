#ifndef BEAM_UDP_SOCKET_READER_HPP
#define BEAM_UDP_SOCKET_READER_HPP
#include "Beam/IO/Reader.hpp"
#include "Beam/Network/UdpSocket.hpp"

namespace Beam {
  template<IsUdpSocketReceiver R>
  class BasicUdpSocketChannel;

  /**
   * Implements the Reader interface for a UDP socket.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicUdpSocketReader {
    public:
      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination);
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size);

    private:
      friend class BasicUdpSocketChannel<R>;
      std::shared_ptr<BasicUdpSocket<R>> m_socket;

      explicit BasicUdpSocketReader(std::shared_ptr<BasicUdpSocket<R>> socket);
      BasicUdpSocketReader(const BasicUdpSocketReader&) = delete;
      BasicUdpSocketReader& operator =(const BasicUdpSocketReader&) = delete;
  };

  /** The on-demand UdpSocketReader type. */
  using UdpSocketReader = BasicUdpSocketReader<UdpSocketReceiver>;

  /** The buffered UdpSocketReader type. */
  using BufferedUdpSocketReader =
    BasicUdpSocketReader<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  bool BasicUdpSocketReader<R>::poll() const {
    return m_socket->get_receiver().poll();
  }

  template<IsUdpSocketReceiver R>
  template<IsBuffer B>
  std::size_t BasicUdpSocketReader<R>::read(Out<B> destination) {
    return read(out(destination), std::size_t(-1));
  }

  template<IsUdpSocketReceiver R>
  template<IsBuffer B>
  std::size_t BasicUdpSocketReader<R>::read(
      Out<B> destination, std::size_t size) {
    return m_socket->get_receiver().receive(out(destination), size);
  }

  template<IsUdpSocketReceiver R>
  BasicUdpSocketReader<R>::BasicUdpSocketReader(
    std::shared_ptr<BasicUdpSocket<R>> socket)
    : m_socket(std::move(socket)) {}
}

#endif
