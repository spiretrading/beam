#ifndef BEAM_UDP_SOCKET_CONNECTION_HPP
#define BEAM_UDP_SOCKET_CONNECTION_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/UdpSocket.hpp"

namespace Beam {

  /** Provides a Connection interface for a UdpSocket. */
  class UdpSocketConnection {
    public:
      ~UdpSocketConnection();

      void close();

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketConnection(std::shared_ptr<UdpSocket> socket);
      UdpSocketConnection(const UdpSocketConnection&) = delete;
      UdpSocketConnection& operator =(const UdpSocketConnection&) = delete;
  };

  inline UdpSocketConnection::~UdpSocketConnection() {
    close();
  }

  inline void UdpSocketConnection::close() {
    m_socket->close();
  }

  inline UdpSocketConnection::UdpSocketConnection(
    std::shared_ptr<UdpSocket> socket)
    : m_socket(std::move(socket)) {}
}

#endif
