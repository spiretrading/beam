#ifndef BEAM_UDPSOCKETCONNECTION_HPP
#define BEAM_UDPSOCKETCONNECTION_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/UdpSocket.hpp"

namespace Beam {
namespace Network {

  /*! \class UdpSocketConnection
      \brief Provides a Connection interface for a UdpSocket.
   */
  class UdpSocketConnection : private boost::noncopyable {
    public:
      ~UdpSocketConnection();

      void Open();

      void Close();

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketConnection(const std::shared_ptr<UdpSocket>& socket);
  };

  inline UdpSocketConnection::~UdpSocketConnection() {
    Close();
  }

  inline void UdpSocketConnection::Open() {
    m_socket->Open();
  }

  inline void UdpSocketConnection::Close() {
    m_socket->Close();
  }

  inline UdpSocketConnection::UdpSocketConnection(
      const std::shared_ptr<UdpSocket>& socket)
      : m_socket(socket) {}
}

  template<>
  struct ImplementsConcept<Network::UdpSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
