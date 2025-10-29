#ifndef BEAM_SECURE_SOCKET_WRITER_HPP
#define BEAM_SECURE_SOCKET_WRITER_HPP
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/TaskRunner.hpp"

namespace Beam {

  /** Writes to an SSL socket. */
  class SecureSocketWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);

    private:
      friend class SecureSocketChannel;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;
      TaskRunner m_tasks;

      SecureSocketWriter(std::shared_ptr<Details::SecureSocketEntry> socket);
      SecureSocketWriter(const SecureSocketWriter&) = delete;
      SecureSocketWriter& operator =(const SecureSocketWriter&) = delete;
  };

  template<IsConstBuffer T>
  void SecureSocketWriter::write(const T& data) {
    auto write_result = Async<void>();
    m_socket->begin_write_operation();
    m_tasks.add([&] {
      auto lock = std::lock_guard(m_socket->m_mutex);
      boost::asio::async_write(m_socket->m_socket,
        boost::asio::buffer(data.get_data(), data.get_size()),
        [&] (const auto& error, auto write_size) {
          if(error) {
            write_result.get_eval().set_exception(
              SocketException(error.value(), error.message()));
          } else {
            write_result.get_eval().set();
          }
        });
    });
    try {
      write_result.get();
      m_socket->end_write_operation();
    } catch(const std::exception&) {
      m_socket->end_write_operation();
      std::throw_with_nested(EndOfFileException());
    }
  }

  inline SecureSocketWriter::SecureSocketWriter(
    std::shared_ptr<Details::SecureSocketEntry> socket)
    : m_socket(std::move(socket)) {}
}

#endif
