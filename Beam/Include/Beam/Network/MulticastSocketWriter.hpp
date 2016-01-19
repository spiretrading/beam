#ifndef BEAM_MULTICASTSOCKETWRITER_HPP
#define BEAM_MULTICASTSOCKETWRITER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/UdpSocketSender.hpp"

namespace Beam {
namespace Network {

  /*! \class MulticastSocketWriter
      \brief Provides the Writer interface to a MulticastSocketSender.
   */
  class MulticastSocketWriter : private boost::noncopyable {
    public:
      using Buffer = IO::SharedBuffer;

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;
      IpAddress m_destination;

      MulticastSocketWriter(const std::shared_ptr<MulticastSocket>& socket,
        const IpAddress& destination);
  };

  inline void MulticastSocketWriter::Write(const void* data, std::size_t size) {
    m_socket->GetSender().Send(data, size, m_destination);
  }

  template<typename BufferType>
  void MulticastSocketWriter::Write(const BufferType& data) {
    Write(data.GetData(), data.GetSize());
  }

  inline MulticastSocketWriter::MulticastSocketWriter(
      const std::shared_ptr<MulticastSocket>& socket,
      const IpAddress& destination)
      : m_socket(socket),
        m_destination(destination) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::MulticastSocketWriter,
      IO::Writer<BufferType>> : std::true_type {};
}

#endif
