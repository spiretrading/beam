module;
#include "Prelude.hpp"

export module Beam:TcpSocketWriter;

import :SocketException;
import :NetworkDetails;

export namespace Beam {

  /** Writes to a TCP socket. */
  class TcpSocketWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);

    private:
      friend class TcpSocketChannel;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;
      TaskRunner m_tasks;

      TcpSocketWriter(std::shared_ptr<Details::TcpSocketEntry> socket);
      TcpSocketWriter(const TcpSocketWriter&) = delete;
      TcpSocketWriter& operator =(const TcpSocketWriter&) = delete;
  };

  template<IsConstBuffer T>
  void TcpSocketWriter::write(const T& data) {
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

  inline TcpSocketWriter::TcpSocketWriter(
    std::shared_ptr<Details::TcpSocketEntry> socket)
    : m_socket(std::move(socket)) {}
}

