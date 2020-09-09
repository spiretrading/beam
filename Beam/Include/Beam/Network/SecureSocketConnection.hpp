#ifndef BEAM_SECURE_SOCKET_CONNECTION_HPP
#define BEAM_SECURE_SOCKET_CONNECTION_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SecureSocketOptions.hpp"

namespace Beam {
namespace Network {

  /** Implements a Connection using an SSL socket over TCP. */
  class SecureSocketConnection {
    public:
      ~SecureSocketConnection();

      void Close();

    private:
      friend class SecureSocketChannel;
      friend class SecureServerSocket;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;
      IO::OpenState m_openState;

      SecureSocketConnection(
        std::shared_ptr<Details::SecureSocketEntry> socket);
      SecureSocketConnection(std::shared_ptr<Details::SecureSocketEntry> socket,
        const SecureSocketOptions& options, const IpAddress& address);
      SecureSocketConnection(std::shared_ptr<Details::SecureSocketEntry> socket,
        const SecureSocketOptions& options, const IpAddress& address,
        const IpAddress& interface);
      SecureSocketConnection(std::shared_ptr<Details::SecureSocketEntry> socket,
        const SecureSocketOptions& options,
        const std::vector<IpAddress>& addresses);
      SecureSocketConnection(std::shared_ptr<Details::SecureSocketEntry> socket,
        const SecureSocketOptions& options,
        const std::vector<IpAddress>& addresses, const IpAddress& interface);
      SecureSocketConnection(const SecureSocketConnection&) = delete;
      SecureSocketConnection& operator =(
        const SecureSocketConnection&) = delete;
      void Open(const SecureSocketOptions& options,
        const std::vector<IpAddress>& addresses,
        const boost::optional<IpAddress>& interface);
  };

  inline SecureSocketConnection::~SecureSocketConnection() {
    Close();
  }

  inline void SecureSocketConnection::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_socket->Close();
    m_openState.Close();
  }

  inline SecureSocketConnection::SecureSocketConnection(
    std::shared_ptr<Details::SecureSocketEntry> socket)
    : m_socket(std::move(socket)) {}

  inline SecureSocketConnection::SecureSocketConnection(
    std::shared_ptr<Details::SecureSocketEntry> socket,
    const SecureSocketOptions& options, const IpAddress& address)
    : SecureSocketConnection(std::move(socket), options,
        std::vector<IpAddress>{address}) {}

  inline SecureSocketConnection::SecureSocketConnection(
    std::shared_ptr<Details::SecureSocketEntry> socket,
    const SecureSocketOptions& options, const IpAddress& address,
    const IpAddress& interface)
    : SecureSocketConnection(std::move(socket), options,
        std::vector<IpAddress>{address}, interface) {}

  inline SecureSocketConnection::SecureSocketConnection(
      std::shared_ptr<Details::SecureSocketEntry> socket,
      const SecureSocketOptions& options,
      const std::vector<IpAddress>& addresses)
      : m_socket(std::move(socket)) {
    Open(options, addresses, boost::none);
  }

  inline SecureSocketConnection::SecureSocketConnection(
      std::shared_ptr<Details::SecureSocketEntry> socket,
      const SecureSocketOptions& options,
      const std::vector<IpAddress>& addresses, const IpAddress& interface)
      : m_socket(std::move(socket)) {
    Open(options, addresses, interface);
  }

  inline void SecureSocketConnection::Open(const SecureSocketOptions& options,
      const std::vector<IpAddress>& addresses,
      const boost::optional<IpAddress>& interface) {
    try {
      m_socket->m_socket.set_verify_mode(boost::asio::ssl::verify_none);
      auto errorCode = boost::system::error_code();
      for(auto& address : addresses) {
        errorCode.clear();
        auto resolver = boost::asio::ip::tcp::resolver(*m_socket->m_ioService);
        auto query = boost::asio::ip::tcp::resolver::query(address.GetHost(),
          std::to_string(address.GetPort()));
        auto end = boost::asio::ip::tcp::resolver::iterator();
        auto endpointIterator = resolver.resolve(query, errorCode);
        if(errorCode) {
          BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
        }
        errorCode = boost::asio::error::host_not_found;
        while(errorCode && endpointIterator != end) {
          auto closeError = boost::system::error_code();
          m_socket->m_socket.lowest_layer().close(closeError);
          if(interface) {
            auto localEndpoint = boost::asio::ip::tcp::endpoint(
              boost::asio::ip::address::from_string(interface->GetHost(),
              errorCode), interface->GetPort());
            if(errorCode) {
              BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
            }
            m_socket->m_socket.lowest_layer().open(boost::asio::ip::tcp::v4(),
              errorCode);
            if(errorCode) {
              BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
            }
            m_socket->m_socket.lowest_layer().bind(localEndpoint, errorCode);
            if(errorCode) {
              BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
            }
          }
          m_socket->m_socket.lowest_layer().connect(*endpointIterator,
            errorCode);
          ++endpointIterator;
        }
        if(!errorCode) {
          break;
        }
      }
      if(errorCode) {
        BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
      }
      auto bufferSize = boost::asio::socket_base::send_buffer_size(
        options.m_writeBufferSize);
      m_socket->m_socket.lowest_layer().set_option(bufferSize, errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
      }
      auto noDelay = boost::asio::ip::tcp::no_delay(options.m_noDelayEnabled);
      m_socket->m_socket.lowest_layer().set_option(noDelay, errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
      }
      m_socket->m_socket.handshake(boost::asio::ssl::stream_base::client,
        errorCode);
      if(errorCode) {
        BOOST_THROW_EXCEPTION(IO::ConnectException(errorCode.message()));
      }
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
    m_socket->m_isOpen = true;
  }
}

  template<>
  struct ImplementsConcept<Network::SecureSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
