#ifndef BEAM_UDP_SOCKET_CONNECTION_HPP
#define BEAM_UDP_SOCKET_CONNECTION_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/UdpSocket.hpp"

namespace Beam {
namespace Network {

  /** Provides a Connection interface for a UdpSocket. */
  class UdpSocketConnection {
    public:
      ~UdpSocketConnection();

      void Close();

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketConnection(std::shared_ptr<UdpSocket> socket);
      UdpSocketConnection(const UdpSocketConnection&) = delete;
      UdpSocketConnection& operator =(const UdpSocketConnection&) = delete;
  };

  inline UdpSocketConnection::~UdpSocketConnection() {
    Close();
  }

  inline void UdpSocketConnection::Close() {
    m_socket->Close();
  }

  inline UdpSocketConnection::UdpSocketConnection(
    std::shared_ptr<UdpSocket> socket)
    : m_socket(std::move(socket)) {}
}

  template<>
  struct ImplementsConcept<Network::UdpSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
