#ifndef BEAM_UDPSOCKETWRITER_HPP
#define BEAM_UDPSOCKETWRITER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketSender.hpp"

namespace Beam {
namespace Network {

  /*! \class UdpSocketWriter
      \brief Provides the Writer interface to a UdpSocketSender.
   */
  class UdpSocketWriter : private boost::noncopyable {
    public:
      using Buffer = IO::SharedBuffer;

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketWriter(const std::shared_ptr<UdpSocket>& socket);
  };

  inline void UdpSocketWriter::Write(const void* data, std::size_t size) {
    m_socket->GetSender().Send(data, size, m_socket->GetAddress());
  }

  template<typename BufferType>
  void UdpSocketWriter::Write(const BufferType& data) {
    Write(data.GetData(), data.GetSize());
  }

  inline UdpSocketWriter::UdpSocketWriter(
      const std::shared_ptr<UdpSocket>& socket)
      : m_socket(socket) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::UdpSocketWriter, IO::Writer<BufferType>> :
    std::true_type {};
}

#endif
