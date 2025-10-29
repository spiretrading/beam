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

  /** Implements a Connection using an SSL socket over TCP. */
  class SecureSocketConnection {
    public:
      ~SecureSocketConnection();

      void close();

    private:
      friend class SecureSocketChannel;
      friend class SecureServerSocket;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;
      OpenState m_open_state;

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
      void open(const SecureSocketOptions& options,
        const std::vector<IpAddress>& addresses,
        const boost::optional<IpAddress>& interface);
  };

  inline SecureSocketConnection::~SecureSocketConnection() {
    close();
  }

  inline void SecureSocketConnection::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_socket->close();
    m_open_state.close();
  }

  inline SecureSocketConnection::SecureSocketConnection(
    std::shared_ptr<Details::SecureSocketEntry> socket)
    : m_socket(std::move(socket)) {}

  inline SecureSocketConnection::SecureSocketConnection(
    std::shared_ptr<Details::SecureSocketEntry> socket,
    const SecureSocketOptions& options, const IpAddress& address)
    : SecureSocketConnection(
        std::move(socket), options, std::vector{address}) {}

  inline SecureSocketConnection::SecureSocketConnection(
    std::shared_ptr<Details::SecureSocketEntry> socket,
    const SecureSocketOptions& options, const IpAddress& address,
    const IpAddress& interface)
    : SecureSocketConnection(
        std::move(socket), options, std::vector{address}, interface) {}

  inline SecureSocketConnection::SecureSocketConnection(
      std::shared_ptr<Details::SecureSocketEntry> socket,
      const SecureSocketOptions& options,
      const std::vector<IpAddress>& addresses)
      : m_socket(std::move(socket)) {
    open(options, addresses, boost::none);
  }

  inline SecureSocketConnection::SecureSocketConnection(
      std::shared_ptr<Details::SecureSocketEntry> socket,
      const SecureSocketOptions& options,
      const std::vector<IpAddress>& addresses, const IpAddress& interface)
      : m_socket(std::move(socket)) {
    open(options, addresses, interface);
  }

  inline void SecureSocketConnection::open(const SecureSocketOptions& options,
      const std::vector<IpAddress>& addresses,
      const boost::optional<IpAddress>& interface) {
    try {
      m_socket->m_socket.set_verify_mode(boost::asio::ssl::verify_none);
      auto error_code =
        boost::system::error_code(boost::asio::error::host_not_found);
      for(auto& address : addresses) {
        error_code.clear();
        auto resolver = boost::asio::ip::tcp::resolver(*m_socket->m_io_context);
        auto ends = resolver.resolve(
          address.get_host(), std::to_string(address.get_port()), error_code);
        if(error_code) {
          boost::throw_with_location(
            SocketException(error_code.value(), error_code.message()));
        }
        error_code = boost::asio::error::host_not_found;
        for(auto& end : ends) {
          auto close_error = boost::system::error_code();
          m_socket->m_socket.lowest_layer().close(close_error);
          if(interface) {
            auto local_end = boost::asio::ip::tcp::endpoint(
              boost::asio::ip::make_address(interface->get_host(), error_code),
              interface->get_port());
            if(error_code) {
              boost::throw_with_location(
                SocketException(error_code.value(), error_code.message()));
            }
            m_socket->m_socket.lowest_layer().open(
              boost::asio::ip::tcp::v4(), error_code);
            if(error_code) {
              boost::throw_with_location(
                SocketException(error_code.value(), error_code.message()));
            }
            m_socket->m_socket.lowest_layer().bind(local_end, error_code);
            if(error_code) {
              boost::throw_with_location(
                SocketException(error_code.value(), error_code.message()));
            }
          }
          m_socket->m_socket.lowest_layer().connect(end, error_code);
          if(!error_code) {
            break;
          }
        }
        if(!error_code) {
          break;
        }
      }
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      auto buffer_size =
        boost::asio::socket_base::send_buffer_size(options.m_write_buffer_size);
      m_socket->m_socket.lowest_layer().set_option(buffer_size, error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      auto noDelay = boost::asio::ip::tcp::no_delay(options.m_no_delay_enabled);
      m_socket->m_socket.lowest_layer().set_option(noDelay, error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      m_socket->m_socket.handshake(
        boost::asio::ssl::stream_base::client, error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
    } catch(const ConnectException&) {
      close();
      throw;
    } catch(const std::exception&) {
      close();
      std::throw_with_nested(ConnectException("Unable to open socket."));
    }
    m_socket->m_is_open = true;
  }
}

#endif
