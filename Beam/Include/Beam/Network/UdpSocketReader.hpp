#ifndef BEAM_UDP_SOCKET_READER_HPP
#define BEAM_UDP_SOCKET_READER_HPP
#include "Beam/IO/Reader.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"

namespace Beam {

  /** Implements the Reader interface for a UdpReceiver. */
  class UdpSocketReader {
    public:
      bool poll() const;
      template<IsBuffer R>
      std::size_t read(Out<R> destination, std::size_t size = -1);

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketReader(std::shared_ptr<UdpSocket> socket);
      UdpSocketReader(const UdpSocketReader&) = delete;
      UdpSocketReader& operator =(const UdpSocketReader&) = delete;
  };

  inline bool UdpSocketReader::poll() const {
    auto command = boost::asio::socket_base::bytes_readable(true);
    {
      auto lock = boost::lock_guard(m_socket->m_socket->m_mutex);
      try {
        m_socket->m_socket->m_socket.io_control(command);
      } catch(const std::exception&) {
        return false;
      }
    }
    return command.get() > 0;
  }

  template<IsBuffer R>
  std::size_t UdpSocketReader::read(Out<R> destination, std::size_t size) {
    auto address = m_socket->get_address();
    return m_socket->get_receiver().receive(
      out(destination), size, out(address));
  }

  inline UdpSocketReader::UdpSocketReader(std::shared_ptr<UdpSocket> socket)
    : m_socket(std::move(socket)) {}
}

#endif
