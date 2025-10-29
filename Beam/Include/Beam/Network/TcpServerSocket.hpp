#ifndef BEAM_TCP_SERVER_SOCKET_HPP
#define BEAM_TCP_SERVER_SOCKET_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"

namespace Beam {

  /** Implements a TCP server socket. */
  class TcpServerSocket {
    public:
      using Channel = TcpSocketChannel;

      /** Constructs a TcpServerSocket. */
      TcpServerSocket();

      /**
       * Constructs a TcpServerSocket.
       * @param options The set of TcpSocketOptions to apply.
       */
      explicit TcpServerSocket(const TcpSocketOptions& options);

      /**
       * Constructs a TcpServerSocket.
       * @param interface The interface to bind to.
       */
      explicit TcpServerSocket(const IpAddress& interface);

      /**
       * Constructs a TcpServerSocket.
       * @param interface The interface to bind to.
       * @param options The set of TcpSocketOptions to apply.
       */
      TcpServerSocket(
        const IpAddress& interface, const TcpSocketOptions& options);

      ~TcpServerSocket();

      std::unique_ptr<Channel> accept();
      void close();

    private:
      TcpSocketOptions m_options;
      boost::asio::io_context* m_io_context;
      boost::optional<boost::asio::ip::tcp::acceptor> m_acceptor;
      OpenState m_open_state;

      TcpServerSocket(const TcpServerSocket&) = delete;
      TcpServerSocket& operator =(const TcpServerSocket&) = delete;
  };

  inline TcpServerSocket::TcpServerSocket()
    : TcpServerSocket(TcpSocketOptions()) {}

  inline TcpServerSocket::TcpServerSocket(const TcpSocketOptions& options)
    : TcpServerSocket(IpAddress("0.0.0.0", 0), options) {}

  inline TcpServerSocket::TcpServerSocket(const IpAddress& interface)
    : TcpServerSocket(interface, TcpSocketOptions()) {}

  inline TcpServerSocket::TcpServerSocket(
      const IpAddress& interface, const TcpSocketOptions& options)
      : m_options(options),
        m_io_context(&ServiceThreadPool::get().get_context()) {
    try {
      auto resolver = boost::asio::ip::tcp::resolver(*m_io_context);
      auto error_code = boost::system::error_code();
      auto ends = resolver.resolve(
        interface.get_host(), std::to_string(interface.get_port()), error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      if(ends.empty()) {
        boost::throw_with_location(SocketException(
          boost::asio::error::invalid_argument, "Invalid interface."));
      }
      m_acceptor.emplace(*m_io_context, *ends.begin());
    } catch(const boost::system::system_error& e) {
      close();
      try {
        boost::throw_with_location(
          SocketException(e.code().value(), e.code().message()));
      } catch(const std::exception&) {
        std::throw_with_nested(ConnectException("Unable to open server."));
      }
    } catch(const std::exception&) {
      close();
      std::throw_with_nested(ConnectException("Unable to open server."));
    }
  }

  inline TcpServerSocket::~TcpServerSocket() {
    close();
  }

  inline std::unique_ptr<TcpServerSocket::Channel> TcpServerSocket::accept() {
    m_open_state.ensure_open();
    auto accept_async = Async<void>();
    auto accept_eval = accept_async.get_eval();
    auto channel = std::unique_ptr<TcpSocketChannel>(new TcpSocketChannel());
    auto accept_callback =
      std::function<void (const boost::system::error_code&)>();
    accept_callback = [&] (const auto& error) {
      if(error) {
        if(error.value() == boost::system::errc::operation_canceled) {
          accept_eval.set_exception(EndOfFileException());
        } else {
#ifdef _WIN32
          if(error.value() == 995) {
            accept_eval.set_exception(EndOfFileException());
            return;
          }
#endif
          accept_eval.set_exception(
            SocketException(error.value(), error.message()));
        }
        return;
      }
      try {
        auto address = IpAddress(
          channel->m_socket->m_socket.remote_endpoint().address().to_string(),
          channel->m_socket->m_socket.remote_endpoint().port());
        channel->set(address);
        channel->get_connection().open(m_options, {}, boost::none);
        accept_eval.set();
      } catch(const std::exception&) {
        channel.reset(new TcpSocketChannel());
        m_acceptor->async_accept(channel->m_socket->m_socket, accept_callback);
      }
    };
    m_acceptor->async_accept(channel->m_socket->m_socket, accept_callback);
    accept_async.get();
    return channel;
  }

  inline void TcpServerSocket::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    if(m_acceptor) {
      m_acceptor->close();
    }
    m_open_state.close();
  }
}

#endif
