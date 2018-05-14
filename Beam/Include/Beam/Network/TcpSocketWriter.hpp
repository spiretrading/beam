#ifndef BEAM_TCPSOCKETWRITER_HPP
#define BEAM_TCPSOCKETWRITER_HPP
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/TaskRunner.hpp"

namespace Beam {
namespace Network {

  /*! \class TcpSocketWriter
      \brief Writes to a TCP socket.
   */
  class TcpSocketWriter : private boost::noncopyable {
    public:
      using Buffer = IO::SharedBuffer;

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      friend class TcpSocketChannel;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;
      Threading::TaskRunner m_tasks;

      TcpSocketWriter(const std::shared_ptr<Details::TcpSocketEntry>& socket);
  };

  inline void TcpSocketWriter::Write(const void* data, std::size_t size) {
    Routines::Async<void> writeResult;
    m_socket->BeginWriteOperation();
    m_tasks.Add(
      [&] {
        boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
        boost::asio::async_write(m_socket->m_socket,
            boost::asio::buffer(data, size),
          [&] (const boost::system::error_code& error, std::size_t writeSize) {
            if(error) {
              if(Details::IsEndOfFile(error)) {
                writeResult.GetEval().SetException(IO::EndOfFileException());
              } else {
                writeResult.GetEval().SetException(
                  SocketException{error.value(), error.message()});
              }
            } else {
              writeResult.GetEval().SetResult();
            }
          });
      });
    try {
      writeResult.Get();
      m_socket->EndWriteOperation();
    } catch(...) {
      m_socket->EndWriteOperation();
      BOOST_RETHROW;
    }
  }

  template<typename BufferType>
  void TcpSocketWriter::Write(const BufferType& data) {
    Write(data.GetData(), data.GetSize());
  }

  inline TcpSocketWriter::TcpSocketWriter(
      const std::shared_ptr<Details::TcpSocketEntry>& socket)
      : m_socket{socket} {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::TcpSocketWriter, IO::Writer<BufferType>> :
    std::true_type {};
}

#endif
