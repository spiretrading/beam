#ifndef BEAM_TCP_SERVER_SOCKET_HPP
#define BEAM_TCP_SERVER_SOCKET_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace Network {

  /** Implements a TCP server socket. */
  class TcpServerSocket {
    public:
      using Channel = TcpSocketChannel;

      /**
       * Constructs a TcpServerSocket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpServerSocket(Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpServerSocket.
       * @param options The set of TcpSocketOptions to apply.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpServerSocket(const TcpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpServerSocket.
       * @param interface The interface to bind to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpServerSocket(const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpServerSocket.
       * @param interface The interface to bind to.
       * @param options The set of TcpSocketOptions to apply.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpServerSocket(const IpAddress& interface,
        const TcpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      ~TcpServerSocket();

      std::unique_ptr<Channel> Accept();

      void Close();

    private:
      TcpSocketOptions m_options;
      SocketThreadPool* m_socketThreadPool;
      boost::asio::io_service* m_ioService;
      boost::optional<boost::asio::ip::tcp::acceptor> m_acceptor;
      IO::OpenState m_openState;

      TcpServerSocket(const TcpServerSocket&) = delete;
      TcpServerSocket& operator =(const TcpServerSocket&) = delete;
  };

  inline TcpServerSocket::TcpServerSocket(
    Ref<SocketThreadPool> socketThreadPool)
    : TcpServerSocket(TcpSocketOptions(), Ref(socketThreadPool)) {}

  inline TcpServerSocket::TcpServerSocket(const TcpSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : TcpServerSocket(IpAddress("0.0.0.0", 0), options,
        Ref(socketThreadPool)) {}

  inline TcpServerSocket::TcpServerSocket(const IpAddress& interface,
    Ref<SocketThreadPool> socketThreadPool)
    : TcpServerSocket(interface, TcpSocketOptions(), Ref(socketThreadPool)) {}

  inline TcpServerSocket::TcpServerSocket(const IpAddress& interface,
      const TcpSocketOptions& options, Ref<SocketThreadPool> socketThreadPool)
      : m_options(options),
        m_socketThreadPool(socketThreadPool.Get()),
        m_ioService(&m_socketThreadPool->GetService()) {
    try {
      auto resolver = boost::asio::ip::tcp::resolver(*m_ioService);
      auto query = boost::asio::ip::tcp::resolver::query(interface.GetHost(),
        std::to_string(interface.GetPort()));
      auto error = boost::system::error_code();
      auto endpointIterator = resolver.resolve(query, error);
      if(error) {
        BOOST_THROW_EXCEPTION(SocketException(error.value(), error.message()));
      }
      m_acceptor.emplace(*m_ioService, *endpointIterator);
    } catch(const SocketException&) {
      Close();
      BOOST_RETHROW;
    } catch(const boost::system::system_error& e) {
      Close();
      BOOST_THROW_EXCEPTION(
        SocketException(e.code().value(), e.code().message()));
    } catch(const std::exception& e) {
      Close();
      BOOST_THROW_EXCEPTION(IO::ConnectException(e.what()));
    }
  }

  inline TcpServerSocket::~TcpServerSocket() {
    Close();
  }

  inline std::unique_ptr<typename TcpServerSocket::Channel>
      TcpServerSocket::Accept() {
    auto acceptAsync = Routines::Async<void>();
    auto acceptEval = acceptAsync.GetEval();
    auto channel = std::unique_ptr<Channel>(new TcpSocketChannel(
      Ref(*m_socketThreadPool)));
    m_acceptor->async_accept(channel->m_socket->m_socket,
      [&] (const auto& error) {
        if(error) {
          if(Details::IsEndOfFile(error)) {
            acceptEval.SetException(IO::EndOfFileException(error.message()));
          } else {
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
          acceptEval.SetException(std::current_exception());
        }
      });
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
