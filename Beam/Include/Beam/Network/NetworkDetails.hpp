#ifndef BEAM_NETWORK_DETAILS_HPP
#define BEAM_NETWORK_DETAILS_HPP
#include <memory>
#include <utility>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::Network::Details {
  template<typename S>
  struct SocketEntry {
    using Socket = S;
    mutable Threading::Mutex m_mutex;
    boost::asio::io_service* m_ioService;
    Socket m_socket;
    bool m_isOpen;
    bool m_isReadPending;
    int m_pendingWrites;
    Threading::ConditionVariable m_isPendingCondition;

    template<typename... Args>
    SocketEntry(boost::asio::io_service& ioService, Args&&... args)
      : m_ioService(&ioService),
        m_socket(std::forward<Args>(args)...),
        m_isOpen(false),
        m_isReadPending(false),
        m_pendingWrites(0) {}

    void Close() {
      auto errorCode = boost::system::error_code();
      auto lock = std::unique_lock(m_mutex);
      if(!m_isOpen) {
        return;
      }
      m_isOpen = false;
      m_socket.shutdown(Socket::shutdown_both, errorCode);
      m_socket.close(errorCode);
      while(m_isReadPending) {
        m_isPendingCondition.wait(lock);
      }
      while(m_pendingWrites != 0) {
        m_isPendingCondition.wait(lock);
      }
    }

    void EndReadOperation() {
      auto lock = std::lock_guard(m_mutex);
      m_isReadPending = false;
      if(!m_isOpen) {
        m_isPendingCondition.notify_all();
      }
    }

    void BeginWriteOperation() {
      auto lock = std::lock_guard(m_mutex);
      if(!m_isOpen) {
        BOOST_THROW_EXCEPTION(IO::EndOfFileException());
      }
      ++m_pendingWrites;
    }

    void EndWriteOperation() {
      auto lock = std::lock_guard(m_mutex);
      --m_pendingWrites;
      if(m_pendingWrites == 0 && !m_isOpen) {
        m_isPendingCondition.notify_all();
      }
    }
  };


  struct SecureSocketEntry {
    using Socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    Threading::Mutex m_mutex;
    boost::asio::io_service* m_ioService;
    boost::asio::ssl::context m_context;
    Socket m_socket;
    bool m_isOpen;
    bool m_isReadPending;
    int m_pendingWrites;
    Threading::ConditionVariable m_isPendingCondition;

    template<typename... Args>
    SecureSocketEntry(boost::asio::io_service& ioService, Args&&... args)
      : m_ioService(&ioService),
        m_context(boost::asio::ssl::context::sslv23),
        m_socket(std::forward<Args>(args)..., m_context),
        m_isOpen(false),
        m_isReadPending(false),
        m_pendingWrites(0) {}

    void Close() {
      auto errorCode = boost::system::error_code();
      auto lock = std::unique_lock(m_mutex);
      if(!m_isOpen) {
        return;
      }
      m_isOpen = false;
      m_socket.lowest_layer().shutdown(
        boost::asio::ip::tcp::socket::shutdown_both, errorCode);
      m_socket.lowest_layer().close(errorCode);
      while(m_isReadPending) {
        m_isPendingCondition.wait(lock);
      }
      while(m_pendingWrites != 0) {
        m_isPendingCondition.wait(lock);
      }
    }

    void EndReadOperation() {
      auto lock = std::lock_guard(m_mutex);
      m_isReadPending = false;
      if(!m_isOpen) {
        m_isPendingCondition.notify_all();
      }
    }

    void BeginWriteOperation() {
      auto lock = std::lock_guard(m_mutex);
      if(!m_isOpen) {
        BOOST_THROW_EXCEPTION(IO::EndOfFileException());
      }
      ++m_pendingWrites;
    }

    void EndWriteOperation() {
      auto lock = std::lock_guard(m_mutex);
      --m_pendingWrites;
      if(m_pendingWrites == 0 && !m_isOpen) {
        m_isPendingCondition.notify_all();
      }
    }
  };

  using TcpSocketEntry = SocketEntry<boost::asio::ip::tcp::socket>;
  using UdpSocketEntry = SocketEntry<boost::asio::ip::udp::socket>;

  inline bool IsEndOfFile(const boost::system::error_code& error) {
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
