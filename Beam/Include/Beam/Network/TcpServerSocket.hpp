#ifndef BEAM_TCP_SERVER_SOCKET_HPP
#define BEAM_TCP_SERVER_SOCKET_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"

namespace Beam {
namespace Network {

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
      TcpServerSocket(const TcpSocketOptions& options);

      /**
       * Constructs a TcpServerSocket.
       * @param interface The interface to bind to.
       */
      TcpServerSocket(const IpAddress& interface);

      /**
       * Constructs a TcpServerSocket.
       * @param interface The interface to bind to.
       * @param options The set of TcpSocketOptions to apply.
       */
      TcpServerSocket(const IpAddress& interface,
        const TcpSocketOptions& options);

      ~TcpServerSocket();

      std::unique_ptr<Channel> Accept();

      void Close();

    private:
      TcpSocketOptions m_options;
      boost::asio::io_context* m_ioContext;
      boost::optional<boost::asio::ip::tcp::acceptor> m_acceptor;
      IO::OpenState m_openState;

      TcpServerSocket(const TcpServerSocket&) = delete;
      TcpServerSocket& operator =(const TcpServerSocket&) = delete;
  };

  inline TcpServerSocket::TcpServerSocket()
    : TcpServerSocket(TcpSocketOptions()) {}

  inline TcpServerSocket::TcpServerSocket(const TcpSocketOptions& options)
    : TcpServerSocket(IpAddress("0.0.0.0", 0), options) {}

  inline TcpServerSocket::TcpServerSocket(const IpAddress& interface)
    : TcpServerSocket(interface, TcpSocketOptions()) {}

  inline TcpServerSocket::TcpServerSocket(const IpAddress& interface,
      const TcpSocketOptions& options)
      : m_options(options),
        m_ioContext(&Threading::ServiceThreadPool::GetInstance().GetContext()) {
    try {
      auto resolver = boost::asio::ip::tcp::resolver(*m_ioContext);
      auto errorCode = boost::system::error_code();
      auto endpoints = resolver.resolve(
        interface.GetHost(), std::to_string(interface.GetPort()), errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(
          SocketException(errorCode.value(), errorCode.message()));
      }
      if(endpoints.empty()) {
        BOOST_THROW_EXCEPTION(SocketException(
          boost::asio::error::invalid_argument, "Invalid interface."));
      }
      m_acceptor.emplace(*m_ioContext, *endpoints.begin());
    } catch(const boost::system::system_error& e) {
      Close();
      try {
        throw SocketException(e.code().value(), e.code().message());
      } catch(const std::exception&) {
        std::throw_with_nested(IO::ConnectException("Unable to open server."));
      }
    } catch(const std::exception&) {
      Close();
      std::throw_with_nested(IO::ConnectException("Unable to open server."));
    }
  }

  inline TcpServerSocket::~TcpServerSocket() {
    Close();
  }

  inline std::unique_ptr<typename TcpServerSocket::Channel>
      TcpServerSocket::Accept() {
    m_openState.EnsureOpen();
    auto acceptAsync = Routines::Async<void>();
    auto acceptEval = acceptAsync.GetEval();
    auto channel = std::unique_ptr<Channel>(new TcpSocketChannel());
    auto acceptCallback = std::function<
      void (const boost::system::error_code&)>();
    acceptCallback = [&] (const auto& error) {
      if(error) {
        if(error.value() == boost::system::errc::operation_canceled) {
          acceptEval.SetException(IO::EndOfFileException());
        } else {
#ifdef _WIN32
          if(error.value() == 995) {
            acceptEval.SetException(IO::EndOfFileException());
            return;
          }
#endif
          acceptEval.SetException(SocketException(error.value(),
            error.message()));
        }
        return;
      }
      try {
        auto address = IpAddress(
          channel->m_socket->m_socket.remote_endpoint().address().to_string(),
          channel->m_socket->m_socket.remote_endpoint().port());
        channel->SetAddress(address);
        channel->GetConnection().Open(m_options, {}, boost::none);
        acceptEval.SetResult();
      } catch(const std::exception&) {
        channel.reset(new TcpSocketChannel());
        m_acceptor->async_accept(channel->m_socket->m_socket, acceptCallback);
      }
    };
    m_acceptor->async_accept(channel->m_socket->m_socket, acceptCallback);
    acceptAsync.Get();
    return channel;
  }

  inline void TcpServerSocket::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    if(m_acceptor) {
      m_acceptor->close();
    }
    m_openState.Close();
  }
}

  template<>
  struct ImplementsConcept<Network::TcpServerSocket,
    IO::ServerConnection<Network::TcpServerSocket::Channel>> :
    std::true_type {};
}

#endif
