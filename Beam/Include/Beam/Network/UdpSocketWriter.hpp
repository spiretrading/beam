#ifndef BEAM_UDP_SOCKET_WRITER_HPP
#define BEAM_UDP_SOCKET_WRITER_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketSender.hpp"

namespace Beam {
  template<IsUdpSocketReceiver R>
  class BasicUdpSocketChannel;

  /**
   * Provides the Writer interface to a UdpSocketSender.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicUdpSocketWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);

    private:
      friend class BasicUdpSocketChannel<R>;
      std::shared_ptr<BasicUdpSocket<R>> m_socket;
      boost::asio::ip::udp::endpoint m_destination;

      explicit BasicUdpSocketWriter(std::shared_ptr<BasicUdpSocket<R>> socket);
      BasicUdpSocketWriter(const BasicUdpSocketWriter&) = delete;
      BasicUdpSocketWriter& operator =(const BasicUdpSocketWriter&) = delete;
  };

  /** The on-demand UdpSocketWriter type. */
  using UdpSocketWriter = BasicUdpSocketWriter<UdpSocketReceiver>;

  /** The buffered UdpSocketWriter type. */
  using BufferedUdpSocketWriter =
    BasicUdpSocketWriter<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  template<IsConstBuffer T>
  void BasicUdpSocketWriter<R>::write(const T& data) {
    m_socket->get_sender().send(data, m_destination);
  }

  template<IsUdpSocketReceiver R>
  BasicUdpSocketWriter<R>::BasicUdpSocketWriter(
    std::shared_ptr<BasicUdpSocket<R>> socket)
    : m_socket(std::move(socket)),
      m_destination(
        boost::asio::ip::make_address(m_socket->get_address().get_host()),
          m_socket->get_address().get_port()) {}
}

#endif
