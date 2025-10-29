#ifndef BEAM_NETWORK_DETAILS_HPP
#define BEAM_NETWORK_DETAILS_HPP
#include <memory>
#include <mutex>
#include <utility>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::Details {
  template<typename S>
  struct SocketEntry {
    using Socket = S;
    mutable Mutex m_mutex;
    boost::asio::io_context* m_io_context;
    Socket m_socket;
    bool m_is_open;
    bool m_is_read_pending;
    int m_pending_writes;
    ConditionVariable m_is_pending_condition;

    template<typename... Args>
    SocketEntry(boost::asio::io_context& ioContext, Args&&... args)
      : m_io_context(&ioContext),
        m_socket(std::forward<Args>(args)...),
        m_is_open(false),
        m_is_read_pending(false),
        m_pending_writes(0) {}

    void close() {
      auto error_code = boost::system::error_code();
      auto lock = std::unique_lock(m_mutex);
      if(!m_is_open) {
        return;
      }
      m_is_open = false;
      m_socket.shutdown(Socket::shutdown_both, error_code);
      m_socket.close(error_code);
      while(m_is_read_pending) {
        m_is_pending_condition.wait(lock);
      }
      while(m_pending_writes != 0) {
        m_is_pending_condition.wait(lock);
      }
    }

    void end_read_operation() {
      auto lock = std::lock_guard(m_mutex);
      m_is_read_pending = false;
      if(!m_is_open) {
        m_is_pending_condition.notify_all();
      }
    }

    void begin_write_operation() {
      auto lock = std::lock_guard(m_mutex);
      if(!m_is_open) {
        boost::throw_with_location(EndOfFileException());
      }
      ++m_pending_writes;
    }

    void end_write_operation() {
      auto lock = std::lock_guard(m_mutex);
      --m_pending_writes;
      if(m_pending_writes == 0 && !m_is_open) {
        m_is_pending_condition.notify_all();
      }
    }
  };


  struct SecureSocketEntry {
    using Socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    Mutex m_mutex;
    boost::asio::io_context* m_io_context;
    boost::asio::ssl::context m_context;
    Socket m_socket;
    bool m_is_open;
    bool m_is_read_pending;
    int m_pending_writes;
    ConditionVariable m_is_pending_condition;

    template<typename... Args>
    SecureSocketEntry(boost::asio::io_context& ioContext, Args&&... args)
      : m_io_context(&ioContext),
        m_context(boost::asio::ssl::context::sslv23),
        m_socket(std::forward<Args>(args)..., m_context),
        m_is_open(false),
        m_is_read_pending(false),
        m_pending_writes(0) {}

    void close() {
      auto error_code = boost::system::error_code();
      auto lock = std::unique_lock(m_mutex);
      if(!m_is_open) {
        return;
      }
      m_is_open = false;
      m_socket.lowest_layer().shutdown(
        boost::asio::ip::tcp::socket::shutdown_both, error_code);
      m_socket.lowest_layer().close(error_code);
      while(m_is_read_pending) {
        m_is_pending_condition.wait(lock);
      }
      while(m_pending_writes != 0) {
        m_is_pending_condition.wait(lock);
      }
    }

    void end_read_operation() {
      auto lock = std::lock_guard(m_mutex);
      m_is_read_pending = false;
      if(!m_is_open) {
        m_is_pending_condition.notify_all();
      }
    }

    void begin_write_operation() {
      auto lock = std::lock_guard(m_mutex);
      if(!m_is_open) {
        boost::throw_with_location(EndOfFileException());
      }
      ++m_pending_writes;
    }

    void end_write_operation() {
      auto lock = std::lock_guard(m_mutex);
      --m_pending_writes;
      if(m_pending_writes == 0 && !m_is_open) {
        m_is_pending_condition.notify_all();
      }
    }
  };

  using TcpSocketEntry = SocketEntry<boost::asio::ip::tcp::socket>;
  using UdpSocketEntry = SocketEntry<boost::asio::ip::udp::socket>;

  inline bool is_end_of_file(const boost::system::error_code& error) {
    return error == boost::asio::error::broken_pipe ||
      error == boost::asio::error::connection_aborted ||
      error == boost::asio::error::connection_reset ||
      error == boost::asio::error::eof ||
      error == boost::asio::error::operation_aborted ||
      error == boost::asio::error::shut_down ||
      error == boost::asio::error::timed_out;
  }
}

#endif
