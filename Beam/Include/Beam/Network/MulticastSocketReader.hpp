#ifndef BEAM_MULTICAST_SOCKET_READER_HPP
#define BEAM_MULTICAST_SOCKET_READER_HPP
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"

namespace Beam {
namespace Network {

  /** Implements the Reader interface for a MulticastSocketReceiver. */
  class MulticastSocketReader {
    public:
      using Buffer = IO::SharedBuffer;

      bool IsDataAvailable() const;

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination, std::size_t size);

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;
      IpAddress m_destination;

      MulticastSocketReader(std::shared_ptr<MulticastSocket> socket,
        IpAddress destination);
      MulticastSocketReader(const MulticastSocketReader&) = delete;
      MulticastSocketReader& operator =(const MulticastSocketReader&) = delete;
  };

  inline bool MulticastSocketReader::IsDataAvailable() const {
    auto command = boost::asio::socket_base::bytes_readable(true);
    {
      auto lock = boost::lock_guard(m_socket->m_socket->m_mutex);
      m_socket->m_socket->m_socket.io_control(command);
    }
    return command.get() > 0;
  }

  template<typename BufferType>
  std::size_t MulticastSocketReader::Read(Out<BufferType> destination) {
    return m_socket->GetReceiver().Receive(Store(destination),
      Store(m_destination));
  }

  inline std::size_t MulticastSocketReader::Read(char* destination,
      std::size_t size) {
    return m_socket->GetReceiver().Receive(destination, size,
      Store(m_destination));
  }

  template<typename BufferType>
  std::size_t MulticastSocketReader::Read(Out<BufferType> destination,
      std::size_t size) {
    return m_socket->GetReceiver().Receive(Store(destination), size,
      Store(m_destination));
  }

  inline MulticastSocketReader::MulticastSocketReader(
    std::shared_ptr<MulticastSocket> socket, IpAddress destination)
    : m_socket(std::move(socket)),
      m_destination(std::move(destination)) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::MulticastSocketReader,
    IO::Reader<BufferType>> : std::true_type {};
}

#endif
