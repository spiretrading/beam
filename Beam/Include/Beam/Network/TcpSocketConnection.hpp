#ifndef BEAM_TCP_SOCKET_CONNECTION_HPP
#define BEAM_TCP_SOCKET_CONNECTION_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/TcpSocketOptions.hpp"

namespace Beam {

  /** Implements a Connection using a TCP socket. */
  class TcpSocketConnection {
    public:
      ~TcpSocketConnection();

      void close();

    private:
      friend class TcpSocketChannel;
      friend class TcpServerSocket;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;
      OpenState m_open_state;

      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options, const IpAddress& address);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options, const IpAddress& address,
        const IpAddress& interface);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options,
        const std::vector<IpAddress>& addresses);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options,
        const std::vector<IpAddress>& addresses, const IpAddress& interface);
      TcpSocketConnection(const TcpSocketConnection&) = delete;
      TcpSocketConnection& operator =(const TcpSocketConnection&) = delete;
      void open(const TcpSocketOptions& options,
        const std::vector<IpAddress>& addresses,
        const boost::optional<IpAddress>& interface);
  };

  inline TcpSocketConnection::~TcpSocketConnection() {
    close();
  }

  inline void TcpSocketConnection::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_socket->close();
    m_open_state.close();
  }

  inline TcpSocketConnection::TcpSocketConnection(
    std::shared_ptr<Details::TcpSocketEntry> socket)
    : m_socket(std::move(socket)) {}

  inline TcpSocketConnection::TcpSocketConnection(
    std::shared_ptr<Details::TcpSocketEntry> socket,
    const TcpSocketOptions& options, const IpAddress& address)
    : TcpSocketConnection(std::move(socket), options, std::vector{address}) {}

  inline TcpSocketConnection::TcpSocketConnection(
    std::shared_ptr<Details::TcpSocketEntry> socket,
    const TcpSocketOptions& options, const IpAddress& address,
    const IpAddress& interface)
    : TcpSocketConnection(
        std::move(socket), options, std::vector{address}, interface) {}

  inline TcpSocketConnection::TcpSocketConnection(
      std::shared_ptr<Details::TcpSocketEntry> socket,
      const TcpSocketOptions& options, const std::vector<IpAddress>& addresses)
      : m_socket(std::move(socket)) {
    open(options, addresses, boost::none);
  }

  inline TcpSocketConnection::TcpSocketConnection(
      std::shared_ptr<Details::TcpSocketEntry> socket,
      const TcpSocketOptions& options, const std::vector<IpAddress>& addresses,
      const IpAddress& interface)
      : m_socket(std::move(socket)) {
    open(options, addresses, interface);
  }

  inline void TcpSocketConnection::open(const TcpSocketOptions& options,
      const std::vector<IpAddress>& addresses,
      const boost::optional<IpAddress>& interface) {
    try {
      auto error_code = [&] {
        if(addresses.empty()) {
          return boost::system::error_code();
        } else {
          return boost::system::error_code(boost::asio::error::host_not_found);
        }
      }();
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
          m_socket->m_socket.close(close_error);
          if(interface) {
            auto local_end = boost::asio::ip::tcp::endpoint(
              boost::asio::ip::make_address(interface->get_host(), error_code),
              interface->get_port());
            if(error_code) {
              boost::throw_with_location(
                SocketException(error_code.value(), error_code.message()));
            }
            m_socket->m_socket.open(boost::asio::ip::tcp::v4(), error_code);
            if(error_code) {
              boost::throw_with_location(
                SocketException(error_code.value(), error_code.message()));
            }
            m_socket->m_socket.bind(local_end, error_code);
            if(error_code) {
              boost::throw_with_location(
                SocketException(error_code.value(), error_code.message()));
            }
          }
          m_socket->m_socket.connect(end, error_code);
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
      m_socket->m_socket.set_option(buffer_size, error_code);
      if(error_code) {
        boost::throw_with_location(
          SocketException(error_code.value(), error_code.message()));
      }
      auto no_delay =
        boost::asio::ip::tcp::no_delay(options.m_no_delay_enabled);
      m_socket->m_socket.set_option(no_delay, error_code);
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
