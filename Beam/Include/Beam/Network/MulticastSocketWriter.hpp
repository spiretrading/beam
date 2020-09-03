#ifndef BEAM_MULTICAST_SOCKET_WRITER_HPP
#define BEAM_MULTICAST_SOCKET_WRITER_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/UdpSocketSender.hpp"

namespace Beam {
namespace Network {

  /** Provides the Writer interface to a MulticastSocketSender. */
  class MulticastSocketWriter {
    public:
      using Buffer = IO::SharedBuffer;

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;
      IpAddress m_destination;

      MulticastSocketWriter(std::shared_ptr<MulticastSocket> socket,
        IpAddress destination);
      MulticastSocketWriter(const MulticastSocketWriter&) = delete;
      MulticastSocketWriter& operator =(const MulticastSocketWriter&) = delete;
  };

  inline void MulticastSocketWriter::Write(const void* data, std::size_t size) {
    m_socket->GetSender().Send(data, size, m_destination);
  }

  template<typename BufferType>
  void MulticastSocketWriter::Write(const BufferType& data) {
    Write(data.GetData(), data.GetSize());
  }

  inline MulticastSocketWriter::MulticastSocketWriter(
    std::shared_ptr<MulticastSocket> socket, IpAddress destination)
    : m_socket(std::move(socket)),
      m_destination(std::move(destination)) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::MulticastSocketWriter,
    IO::Writer<BufferType>> : std::true_type {};
}

#endif
