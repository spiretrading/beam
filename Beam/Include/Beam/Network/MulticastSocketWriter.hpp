#ifndef BEAM_MULTICAST_SOCKET_WRITER_HPP
#define BEAM_MULTICAST_SOCKET_WRITER_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/UdpSocketSender.hpp"

namespace Beam {
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocketChannel;

  /**
   * Provides the Writer interface to a MulticastSocketSender.
   * @tparam R The datagram receiver.
   */
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocketWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);

    private:
      friend class BasicMulticastSocketChannel<R>;
      std::shared_ptr<BasicMulticastSocket<R>> m_socket;
      boost::asio::ip::udp::endpoint m_destination;

      BasicMulticastSocketWriter(
        std::shared_ptr<BasicMulticastSocket<R>> socket, IpAddress destination);
      BasicMulticastSocketWriter(const BasicMulticastSocketWriter&) = delete;
      BasicMulticastSocketWriter& operator =(
        const BasicMulticastSocketWriter&) = delete;
  };

  /** The on-demand MulticastSocketWriter type. */
  using MulticastSocketWriter = BasicMulticastSocketWriter<UdpSocketReceiver>;

  /** The buffered MulticastSocketWriter type. */
  using BufferedMulticastSocketWriter =
    BasicMulticastSocketWriter<BufferedUdpSocketReceiver>;

  template<IsUdpSocketReceiver R>
  template<IsConstBuffer T>
  void BasicMulticastSocketWriter<R>::write(const T& data) {
    m_socket->get_sender().send(data, m_destination);
  }

  template<IsUdpSocketReceiver R>
  BasicMulticastSocketWriter<R>::BasicMulticastSocketWriter(
    std::shared_ptr<BasicMulticastSocket<R>> socket, IpAddress destination)
    : m_socket(std::move(socket)),
      m_destination(boost::asio::ip::make_address(
        destination.get_host()), destination.get_port()) {}
}

#endif
