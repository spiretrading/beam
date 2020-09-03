#ifndef BEAM_MULTICAST_SOCKET_CONNECTION_HPP
#define BEAM_MULTICAST_SOCKET_CONNECTION_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/Network.hpp"

namespace Beam {
namespace Network {

  /** Provides a Connection interface for a MulticastSocket. */
  class MulticastSocketConnection {
    public:
      ~MulticastSocketConnection();

      void Close();

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;

      MulticastSocketConnection(std::shared_ptr<MulticastSocket> socket);
      MulticastSocketConnection(const MulticastSocketConnection&) = delete;
      MulticastSocketConnection& operator =(
        const MulticastSocketConnection&) = delete;
  };

  inline MulticastSocketConnection::~MulticastSocketConnection() {
    Close();
  }

  inline void MulticastSocketConnection::Close() {
    m_socket->Close();
  }

  inline MulticastSocketConnection::MulticastSocketConnection(
    std::shared_ptr<MulticastSocket> socket)
    : m_socket(std::move(socket)) {}
}

  template<>
  struct ImplementsConcept<Network::MulticastSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
