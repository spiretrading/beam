#ifndef BEAM_UDP_SOCKET_READER_HPP
#define BEAM_UDP_SOCKET_READER_HPP
#include "Beam/IO/Reader.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"

namespace Beam {
namespace Network {

  /** Implements the Reader interface for a UdpReceiver. */
  class UdpSocketReader {
    public:
      bool IsDataAvailable() const;

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketReader(std::shared_ptr<UdpSocket> socket);
      UdpSocketReader(const UdpSocketReader&) = delete;
      UdpSocketReader& operator =(const UdpSocketReader&) = delete;
  };

  inline bool UdpSocketReader::IsDataAvailable() const {
    auto command = boost::asio::socket_base::bytes_readable(true);
    {
      auto lock = boost::lock_guard(m_socket->m_socket->m_mutex);
      m_socket->m_socket->m_socket.io_control(command);
    }
    return command.get() > 0;
  }

  template<typename Buffer>
  std::size_t UdpSocketReader::Read(Out<Buffer> destination) {
    auto address = m_socket->GetAddress();
    return m_socket->GetReceiver().Receive(Store(destination), Store(address));
  }

  inline std::size_t UdpSocketReader::Read(char* destination,
      std::size_t size) {
    auto address = m_socket->GetAddress();
    return m_socket->GetReceiver().Receive(destination, size,
      Store(address));
  }

  template<typename Buffer>
  std::size_t UdpSocketReader::Read(Out<Buffer> destination, std::size_t size) {
    auto address = m_socket->GetAddress();
    return m_socket->GetReceiver().Receive(Store(destination), size,
      Store(address));
  }

  inline UdpSocketReader::UdpSocketReader(std::shared_ptr<UdpSocket> socket)
    : m_socket(std::move(socket)) {}
}

  template<>
  struct ImplementsConcept<Network::UdpSocketReader, IO::Reader> :
    std::true_type {};
}

#endif
