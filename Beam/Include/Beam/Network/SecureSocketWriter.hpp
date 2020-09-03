#ifndef BEAM_SECURE_SOCKET_WRITER_HPP
#define BEAM_SECURE_SOCKET_WRITER_HPP
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
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

  /** Writes to an SSL socket. */
  class SecureSocketWriter {
    public:
      using Buffer = IO::SharedBuffer;

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      friend class SecureSocketChannel;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;
      Threading::TaskRunner m_tasks;

      SecureSocketWriter(std::shared_ptr<Details::SecureSocketEntry> socket);
      SecureSocketWriter(const SecureSocketWriter&) = delete;
      SecureSocketWriter& operator =(const SecureSocketWriter&) = delete;
  };

  inline void SecureSocketWriter::Write(const void* data, std::size_t size) {
    auto writeResult = Routines::Async<void>();
    m_socket->BeginWriteOperation();
    m_tasks.Add(
      [&] {
        auto lock = std::lock_guard(m_socket->m_mutex);
        boost::asio::async_write(m_socket->m_socket,
          boost::asio::buffer(data, size),
          [&] (const auto& error, auto writeSize) {
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
    } catch(const std::exception&) {
      m_socket->EndWriteOperation();
      BOOST_RETHROW;
    }
  }

  template<typename BufferType>
  void SecureSocketWriter::Write(const BufferType& data) {
    Write(data.GetData(), data.GetSize());
  }

  inline SecureSocketWriter::SecureSocketWriter(
    std::shared_ptr<Details::SecureSocketEntry> socket)
    : m_socket(std::move(socket)) {}
}

  template<typename BufferType>
  struct ImplementsConcept<Network::SecureSocketWriter,
    IO::Writer<BufferType>> : std::true_type {};
}

#endif
