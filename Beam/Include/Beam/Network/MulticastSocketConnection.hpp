#ifndef BEAM_MULTICASTSOCKETCONNECTION_HPP
#define BEAM_MULTICASTSOCKETCONNECTION_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/Network.hpp"

namespace Beam {
namespace Network {

  /*! \class MulticastSocketConnection
      \brief Provides a Connection interface for a MulticastSocket.
   */
  class MulticastSocketConnection : private boost::noncopyable {
    public:
      ~MulticastSocketConnection();

      void Open();

      void Close();

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;

      MulticastSocketConnection(const std::shared_ptr<MulticastSocket>& socket);
  };

  inline MulticastSocketConnection::~MulticastSocketConnection() {
    Close();
  }

  inline void MulticastSocketConnection::Open() {
    m_socket->Open();
  }

  inline void MulticastSocketConnection::Close() {
    m_socket->Close();
  }

  inline MulticastSocketConnection::MulticastSocketConnection(
      const std::shared_ptr<MulticastSocket>& socket)
      : m_socket(socket) {}
}

  template<>
  struct ImplementsConcept<Network::MulticastSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
