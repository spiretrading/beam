#ifndef BEAM_SECURE_SOCKET_CHANNEL_HPP
#define BEAM_SECURE_SOCKET_CHANNEL_HPP
#include <boost/asio/ip/tcp.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SecureSocketConnection.hpp"
#include "Beam/Network/SecureSocketOptions.hpp"
#include "Beam/Network/SecureSocketReader.hpp"
#include "Beam/Network/SecureSocketWriter.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace Network {

  /** Implements the Channel interface using an SSL socket over TCP. */
  class SecureSocketChannel {
    public:
      using Identifier = SocketIdentifier;
      using Connection = SecureSocketConnection;
      using Reader = SecureSocketReader;
      using Writer = SecureSocketWriter;

      /**
       * Constructs a SecureSocketChannel.
       * @param address The IP address to connect to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const IpAddress& address,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a SecureSocketChannel.
       * @param address The IP address to connect to.
       * @param options The options to apply to the socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const IpAddress& address,
        const SecureSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a SecureSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const IpAddress& address, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a SecureSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       * @param options The options to apply to the socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const IpAddress& address, const IpAddress& interface,
        const SecureSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const std::vector<IpAddress>& addresses,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param options The options to apply to the socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const std::vector<IpAddress>& addresses,
        const SecureSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const std::vector<IpAddress>& addresses,
        const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       * @param options The options to apply to the socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      SecureSocketChannel(const std::vector<IpAddress>& addresses,
        const IpAddress& interface, const SecureSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      friend class SecureServerSocket;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;
      Identifier m_identifier;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      SecureSocketChannel(Ref<SocketThreadPool> socketThreadPool);
      SecureSocketChannel(const SecureSocketChannel&) = delete;
      SecureSocketChannel& operator =(const SecureSocketChannel&) = delete;
      void SetAddress(const IpAddress& address);
  };

  inline SecureSocketChannel::SecureSocketChannel(const IpAddress& address,
    Ref<SocketThreadPool> socketThreadPool)
    : SecureSocketChannel(address, SecureSocketOptions(),
        Ref(socketThreadPool)) {}

  inline SecureSocketChannel::SecureSocketChannel(const IpAddress& address,
    const SecureSocketOptions& options, Ref<SocketThreadPool> socketThreadPool)
    : SecureSocketChannel(std::vector<IpAddress>{address}, options,
        Ref(socketThreadPool)) {}

  inline SecureSocketChannel::SecureSocketChannel(const IpAddress& address,
    const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
    : SecureSocketChannel(address, interface, SecureSocketOptions(),
        Ref(socketThreadPool)) {}

  inline SecureSocketChannel::SecureSocketChannel(const IpAddress& address,
    const IpAddress& interface, const SecureSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : SecureSocketChannel(std::vector<IpAddress>{address}, interface, options,
        Ref(socketThreadPool)) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses,
    Ref<SocketThreadPool> socketThreadPool)
    : SecureSocketChannel(addresses, SecureSocketOptions(),
        Ref(socketThreadPool)) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses, const SecureSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : m_socket(std::make_shared<Details::SecureSocketEntry>(
        socketThreadPool->GetService(), socketThreadPool->GetService())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface,
    Ref<SocketThreadPool> socketThreadPool)
    : SecureSocketChannel(addresses, interface, SecureSocketOptions(),
        Ref(socketThreadPool)) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface,
    const SecureSocketOptions& options, Ref<SocketThreadPool> socketThreadPool)
    : m_socket(std::make_shared<Details::SecureSocketEntry>(
        socketThreadPool->GetService(), socketThreadPool->GetService())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses, interface),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline const SecureSocketChannel::Identifier&
      SecureSocketChannel::GetIdentifier() const {
    return m_identifier;
  }

  inline SecureSocketChannel::Connection& SecureSocketChannel::GetConnection() {
    return m_connection;
  }

  inline SecureSocketChannel::Reader& SecureSocketChannel::GetReader() {
    return m_reader;
  }

  inline SecureSocketChannel::Writer& SecureSocketChannel::GetWriter() {
    return m_writer;
  }

  inline SecureSocketChannel::SecureSocketChannel(
    Ref<SocketThreadPool> socketThreadPool)
    : m_socket(std::make_shared<Details::SecureSocketEntry>(
        socketThreadPool->GetService(), socketThreadPool->GetService())),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline void SecureSocketChannel::SetAddress(const IpAddress& address) {
    m_identifier = SocketIdentifier(address);
  }
}

  template<>
  struct ImplementsConcept<Network::SecureSocketChannel, IO::Channel<
    Network::SecureSocketChannel::Identifier,
    Network::SecureSocketChannel::Connection,
    Network::SecureSocketChannel::Reader,
    Network::SecureSocketChannel::Writer>> : std::true_type {};
}

#endif
