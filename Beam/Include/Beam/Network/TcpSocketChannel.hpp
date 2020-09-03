#ifndef BEAM_TCP_SOCKET_CHANNEL_HPP
#define BEAM_TCP_SOCKET_CHANNEL_HPP
#include <boost/asio/ip/tcp.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Network/TcpSocketConnection.hpp"
#include "Beam/Network/TcpSocketOptions.hpp"
#include "Beam/Network/TcpSocketReader.hpp"
#include "Beam/Network/TcpSocketWriter.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace Network {

  /** Implements the Channel interface using a TCP socket. */
  class TcpSocketChannel {
    public:
      using Identifier = SocketIdentifier;
      using Connection = TcpSocketConnection;
      using Reader = TcpSocketReader;
      using Writer = TcpSocketWriter;

      /**
       * Constructs a TcpSocketChannel.
       * @param address The IP address to connect to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const IpAddress& address,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpSocketChannel.
       * @param address The IP address to connect to.
       * @param options The set of TcpSocketOptions to apply.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const IpAddress& address,
        const TcpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const IpAddress& address, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       * @param options The set of TcpSocketOptions to apply.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const IpAddress& address, const IpAddress& interface,
        const TcpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param options The set of TcpSocketOptions to apply.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        const TcpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       * @param options The set of TcpSocketOptions to apply.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        const IpAddress& interface, const TcpSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

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

      TcpSocketChannel(Ref<SocketThreadPool> socketThreadPool);
      TcpSocketChannel(const TcpSocketChannel&) = delete;
      TcpSocketChannel& operator =(const TcpSocketChannel&) = delete;
      void SetAddress(const IpAddress& address);
  };

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address,
    Ref<SocketThreadPool> socketThreadPool)
    : TcpSocketChannel(address, TcpSocketOptions(), Ref(socketThreadPool)) {}

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address,
    const TcpSocketOptions& options, Ref<SocketThreadPool> socketThreadPool)
    : TcpSocketChannel(std::vector<IpAddress>{address}, options,
        Ref(socketThreadPool)) {}

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address,
    const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
    : TcpSocketChannel(address, interface, TcpSocketOptions(),
        Ref(socketThreadPool)) {}

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address,
    const IpAddress& interface, const TcpSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : TcpSocketChannel(std::vector<IpAddress>{address}, interface, options,
        Ref(socketThreadPool)) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses,
    Ref<SocketThreadPool> socketThreadPool)
    : TcpSocketChannel(addresses, TcpSocketOptions(), Ref(socketThreadPool)) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses, const TcpSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : m_socket(std::make_shared<Details::TcpSocketEntry>(
        socketThreadPool->GetService(), socketThreadPool->GetService())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface,
    Ref<SocketThreadPool> socketThreadPool)
    : TcpSocketChannel(addresses, interface, TcpSocketOptions(),
        Ref(socketThreadPool)) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface,
    const TcpSocketOptions& options, Ref<SocketThreadPool> socketThreadPool)
    : m_socket(std::make_shared<Details::TcpSocketEntry>(
        socketThreadPool->GetService(), socketThreadPool->GetService())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses, interface),
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
    Ref<SocketThreadPool> socketThreadPool)
    : m_socket(std::make_shared<Details::TcpSocketEntry>(
        socketThreadPool->GetService(), socketThreadPool->GetService())),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline void TcpSocketChannel::SetAddress(const IpAddress& address) {
    m_identifier = SocketIdentifier(address);
  }
}

  template<>
  struct ImplementsConcept<Network::TcpSocketChannel, IO::Channel<
    Network::TcpSocketChannel::Identifier,
    Network::TcpSocketChannel::Connection, Network::TcpSocketChannel::Reader,
    Network::TcpSocketChannel::Writer>> : std::true_type {};
}

#endif
