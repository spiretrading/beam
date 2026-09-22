#ifndef BEAM_UDP_SOCKET_WRITER_HPP
#define BEAM_UDP_SOCKET_WRITER_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketSender.hpp"

namespace Beam {

  /** Provides the Writer interface to a UdpSocketSender. */
  class UdpSocketWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;
      boost::asio::ip::udp::endpoint m_destination;

      UdpSocketWriter(std::shared_ptr<UdpSocket> socket);
      UdpSocketWriter(const UdpSocketWriter&) = delete;
      UdpSocketWriter& operator =(const UdpSocketWriter&) = delete;
  };

  template<IsConstBuffer T>
  void UdpSocketWriter::write(const T& data) {
    m_socket->get_sender().send(data, m_destination);
  }

  inline UdpSocketWriter::UdpSocketWriter(std::shared_ptr<UdpSocket> socket)
    : m_socket(std::move(socket)),
      m_destination(
        boost::asio::ip::make_address(m_socket->get_address().get_host()),
          m_socket->get_address().get_port()) {}
}

#endif
