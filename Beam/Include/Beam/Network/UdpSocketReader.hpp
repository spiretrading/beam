#ifndef BEAM_UDPSOCKETREADER_HPP
#define BEAM_UDPSOCKETREADER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"

namespace Beam {
namespace Network {

  /*! \class UdpSocketReader
      \brief Implements the Reader interface for a UdpReceiver.
   */
  class UdpSocketReader : private boost::noncopyable {
    public:
      using Buffer = IO::SharedBuffer;

      bool IsDataAvailable() const;

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination, std::size_t size);

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketReader(const std::shared_ptr<UdpSocket>& socket);
  };

  inline bool UdpSocketReader::IsDataAvailable() const {
    boost::asio::socket_base::bytes_readable command(true);
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_socket->m_mutex};
      m_socket->m_socket->m_socket.io_control(command);
    }
    return command.get() > 0;
  }

  template<typename BufferType>
  std::size_t UdpSocketReader::Read(Out<BufferType> destination) {
    auto address = m_socket->GetAddress();
    return m_socket->GetReceiver().Receive(Store(destination),
      Store(address));
  }

  inline std::size_t UdpSocketReader::Read(char* destination,
      std::size_t size) {
    auto address = m_socket->GetAddress();
    return m_socket->GetReceiver().Receive(destination, size,
      Store(address));
  }

  template<typename BufferType>
  std::size_t UdpSocketReader::Read(Out<BufferType> destination,
      std::size_t size) {
    auto address = m_socket->GetAddress();
    return m_socket->GetReceiver().Receive(Store(destination), size,
      Store(address));
  }

  inline UdpSocketReader::UdpSocketReader(
      const std::shared_ptr<UdpSocket>& socket)
      : m_socket(socket) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::UdpSocketReader, IO::Reader<BufferType>> :
    std::true_type {};
}

#endif
