#ifndef BEAM_SERVERSOCKET_HPP
#define BEAM_SERVERSOCKET_HPP
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Network/TcpSocketConnection.hpp"
#include "Beam/Network/TcpSocketReader.hpp"
#include "Beam/Network/TcpSocketWriter.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
namespace Network {

  /*! \class TcpServerSocket
      \brief Implements a server socket.
   */
  class TcpServerSocket : private boost::noncopyable {
    public:
      using Channel = TcpSocketChannel;

      //! Constructs a TcpServerSocket.
      /*!
        \param address The IP address to bind to.
        \param socketThreadPool The thread pool used for the sockets.
      */
      TcpServerSocket(const IpAddress& address,
        RefType<SocketThreadPool> socketThreadPool);

      ~TcpServerSocket();

      std::unique_ptr<Channel> Accept();

      void Open();

      void Close();

    private:
      IpAddress m_address;
      SocketThreadPool* m_socketThreadPool;
      boost::asio::io_service* m_ioService;
      boost::optional<boost::asio::ip::tcp::acceptor> m_acceptor;
      IO::OpenState m_openState;

      void Shutdown();
  };

  inline TcpServerSocket::TcpServerSocket(const IpAddress& address,
      RefType<SocketThreadPool> socketThreadPool)
      : m_address(address),
        m_socketThreadPool(socketThreadPool.Get()),
        m_ioService(&m_socketThreadPool->GetService()) {}

  inline TcpServerSocket::~TcpServerSocket() {
    Close();
  }

  inline void TcpServerSocket::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      boost::asio::ip::tcp::resolver resolver(*m_ioService);
      boost::asio::ip::tcp::resolver::query query(m_address.GetHost(),
        ToString(m_address.GetPort()));
      boost::system::error_code error;
      auto endpointIterator = resolver.resolve(query, error);
      if(error != 0) {
        BOOST_THROW_EXCEPTION(SocketException(error.value(), error.message()));
      }
      m_acceptor.emplace(*m_ioService, *endpointIterator);
    } catch(const SocketException&) {
      m_openState.SetOpenFailure();
      Shutdown();
    } catch(const boost::system::system_error& e) {
      m_openState.SetOpenFailure(
        SocketException{e.code().value(), e.code().message()});
      Shutdown();
    } catch(const std::exception& e) {
      m_openState.SetOpenFailure(IO::ConnectException{e.what()});
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline std::unique_ptr<typename TcpServerSocket::Channel>
      TcpServerSocket::Accept() {
    Routines::Async<void> acceptAsync;
    Routines::Eval<void> acceptEval = acceptAsync.GetEval();
    std::unique_ptr<Channel> channel(new TcpSocketChannel{
      Ref(*m_socketThreadPool)});
    m_acceptor->async_accept(channel->m_socket->m_socket,
      [&] (const boost::system::error_code& error) {
        if(error != 0) {
          if(Details::IsEndOfFile(error)) {
            acceptEval.SetException(IO::EndOfFileException(error.message()));
          } else {
            acceptEval.SetException(SocketException(error.value(),
              error.message()));
          }
          return;
        }
        try {
          IpAddress address(
            channel->m_socket->m_socket.remote_endpoint().address().to_string(),
            channel->m_socket->m_socket.remote_endpoint().port());
          channel->SetAddress(address);
          channel->GetConnection().SetOpen();
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
    Shutdown();
  }

  inline void TcpServerSocket::Shutdown() {
    if(m_acceptor.is_initialized()) {
      m_acceptor->close();
    }
    m_openState.SetClosed();
  }
}

  template<>
  struct ImplementsConcept<Network::TcpServerSocket,
    IO::ServerConnection<Network::TcpServerSocket::Channel>> :
    std::true_type {};
}

#endif
