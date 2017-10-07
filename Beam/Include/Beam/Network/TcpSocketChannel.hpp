#ifndef BEAM_TCPSOCKETCHANNEL_HPP
#define BEAM_TCPSOCKETCHANNEL_HPP
#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Network/TcpSocketConnection.hpp"
#include "Beam/Network/TcpSocketReader.hpp"
#include "Beam/Network/TcpSocketWriter.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace Network {

  /*! \class TcpSocketChannel
      \brief Implements the Channel interface using a TCP socket.
   */
  class TcpSocketChannel : private boost::noncopyable {
    public:
      using Identifier = SocketIdentifier;
      using Connection = TcpSocketConnection;
      using Reader = TcpSocketReader;
      using Writer = TcpSocketWriter;

      //! Constructs a TcpSocketChannel.
      /*!
        \param address The IP address to connect to.
        \param socketThreadPool The thread pool used for the sockets.
      */
      TcpSocketChannel(const IpAddress& address,
        RefType<SocketThreadPool> socketThreadPool);

      //! Constructs a TcpSocketChannel.
      /*!
        \param address The IP address to connect to.
        \param interface The interface to bind to.
        \param socketThreadPool The thread pool used for the sockets.
      */
      TcpSocketChannel(const IpAddress& address,
        const IpAddress& interface, RefType<SocketThreadPool> socketThreadPool);

      //! Constructs a TcpSocketChannel.
      /*!
        \param addresses The list of IP addresses to try to connect to.
        \param socketThreadPool The thread pool used for the sockets.
      */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        RefType<SocketThreadPool> socketThreadPool);

      //! Constructs a TcpSocketChannel.
      /*!
        \param addresses The list of IP addresses to try to connect to.
        \param interface The interface to bind to.
        \param socketThreadPool The thread pool used for the sockets.
      */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        const IpAddress& interface, RefType<SocketThreadPool> socketThreadPool);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      friend class TcpServerSocket;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;
      Identifier m_identifier;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      TcpSocketChannel(RefType<SocketThreadPool> socketThreadPool);
      void SetAddress(const IpAddress& address);
  };

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address,
      RefType<SocketThreadPool> socketThreadPool)
      : m_socket(std::make_shared<Details::TcpSocketEntry>(
          socketThreadPool->GetService())),
        m_identifier(address),
        m_connection(m_socket, address),
        m_reader(m_socket),
        m_writer(m_socket) {}

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address,
      const IpAddress& interface, RefType<SocketThreadPool> socketThreadPool)
      : m_socket(std::make_shared<Details::TcpSocketEntry>(
          socketThreadPool->GetService())),
        m_identifier(address),
        m_connection(m_socket, address, interface),
        m_reader(m_socket),
        m_writer(m_socket) {}

  inline TcpSocketChannel::TcpSocketChannel(
      const std::vector<IpAddress>& addresses,
      RefType<SocketThreadPool> socketThreadPool)
      : m_socket(std::make_shared<Details::TcpSocketEntry>(
          socketThreadPool->GetService())),
        m_identifier(addresses.front()),
        m_connection(m_socket, addresses),
        m_reader(m_socket),
        m_writer(m_socket) {}

  inline TcpSocketChannel::TcpSocketChannel(
      const std::vector<IpAddress>& addresses,
      const IpAddress& interface, RefType<SocketThreadPool> socketThreadPool)
      : m_socket(std::make_shared<Details::TcpSocketEntry>(
          socketThreadPool->GetService())),
        m_identifier(addresses.front()),
        m_connection(m_socket, addresses, interface),
        m_reader(m_socket),
        m_writer(m_socket) {}

  inline const TcpSocketChannel::Identifier&
      TcpSocketChannel::GetIdentifier() const {
    return m_identifier;
  }

  inline TcpSocketChannel::Connection& TcpSocketChannel::GetConnection() {
    return m_connection;
  }

  inline TcpSocketChannel::Reader& TcpSocketChannel::GetReader() {
    return m_reader;
  }

  inline TcpSocketChannel::Writer& TcpSocketChannel::GetWriter() {
    return m_writer;
  }

  inline TcpSocketChannel::TcpSocketChannel(
      RefType<SocketThreadPool> socketThreadPool)
      : m_socket(std::make_shared<Details::TcpSocketEntry>(
          socketThreadPool->GetService())),
        m_connection(m_socket),
        m_reader(m_socket),
        m_writer(m_socket) {}

  inline void TcpSocketChannel::SetAddress(const IpAddress& address) {
    m_identifier = SocketIdentifier(address);
    std::vector<IpAddress> addresses;
    addresses.push_back(address);
    m_connection.m_addresses = addresses;
  }
}

  template<>
  struct ImplementsConcept<Network::TcpSocketChannel, IO::Channel<
    Network::TcpSocketChannel::Identifier,
    Network::TcpSocketChannel::Connection, Network::TcpSocketChannel::Reader,
    Network::TcpSocketChannel::Writer>> : std::true_type {};
}

#endif
