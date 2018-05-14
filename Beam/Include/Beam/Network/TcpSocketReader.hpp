#ifndef BEAM_TCPSOCKETREADER_HPP
#define BEAM_TCPSOCKETREADER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam {
namespace Network {

  /*! \class TcpSocketReader
      \brief Reads from a TCP socket.
   */
  class TcpSocketReader : private boost::noncopyable {
    public:
      using Buffer = IO::SharedBuffer;

      bool IsDataAvailable() const;

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination, std::size_t size);

    private:
      friend class TcpSocketChannel;
      static const std::size_t DEFAULT_READ_SIZE = 8 * 1024;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;

      TcpSocketReader(const std::shared_ptr<Details::TcpSocketEntry>& socket);
  };

  inline bool TcpSocketReader::IsDataAvailable() const {
    boost::asio::socket_base::bytes_readable command(true);
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      m_socket->m_socket.io_control(command);
    }
    return command.get() > 0;
  }

  template<typename BufferType>
  std::size_t TcpSocketReader::Read(Out<BufferType> destination) {
    return Read(Store(destination), DEFAULT_READ_SIZE);
  }

  inline std::size_t TcpSocketReader::Read(char* destination,
      std::size_t size) {
    Routines::Async<std::size_t> readResult;
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      if(!m_socket->m_isOpen) {
        BOOST_THROW_EXCEPTION(IO::EndOfFileException{});
      }
      m_socket->m_isReadPending = true;
      m_socket->m_socket.async_read_some(boost::asio::buffer(destination, size),
        [&] (const boost::system::error_code& error, std::size_t readSize) {
          if(error) {
            if(Details::IsEndOfFile(error)) {
              readResult.GetEval().SetException(
                IO::EndOfFileException(error.message()));
            } else {
              readResult.GetEval().SetException(SocketException(error.value(),
                error.message()));
            }
          } else {
            readResult.GetEval().SetResult(readSize);
          }
        });
    }
    try {
      auto result = readResult.Get();
      m_socket->EndReadOperation();
      return result;
    } catch(...) {
      m_socket->EndReadOperation();
      BOOST_RETHROW;
    }
  }

  template<typename BufferType>
  std::size_t TcpSocketReader::Read(Out<BufferType> destination,
      std::size_t size) {
    static const auto DEFAULT_READ_SIZE = std::size_t{8 * 1024};
    auto initialSize = destination->GetSize();
    auto readSize = std::min(DEFAULT_READ_SIZE, size);
    destination->Grow(readSize);
    auto result = Read(destination->GetMutableData() + initialSize, readSize);
    destination->Shrink(readSize - result);
    return result;
  }

  inline TcpSocketReader::TcpSocketReader(
      const std::shared_ptr<Details::TcpSocketEntry>& socket)
      : m_socket(socket) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::TcpSocketReader, IO::Reader<BufferType>> :
    std::true_type {};
}

#endif
