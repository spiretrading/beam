#ifndef BEAM_MULTICASTSOCKETREADER_HPP
#define BEAM_MULTICASTSOCKETREADER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"

namespace Beam {
namespace Network {

  /*! \class MulticastSocketReader
      \brief Implements the Reader interface for a MulticastSocketReceiver.
   */
  class MulticastSocketReader : private boost::noncopyable {
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

      MulticastSocketReader(const std::shared_ptr<MulticastSocket>& socket,
        const IpAddress& destination);
  };

  inline bool MulticastSocketReader::IsDataAvailable() const {
    boost::asio::socket_base::bytes_readable command(true);
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_socket->m_mutex};
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
      const std::shared_ptr<MulticastSocket>& socket,
      const IpAddress& destination)
      : m_socket(socket),
        m_destination(destination) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::MulticastSocketReader,
      IO::Reader<BufferType>> : std::true_type {};
}

#endif
