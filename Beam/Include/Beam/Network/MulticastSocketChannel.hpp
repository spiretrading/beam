#ifndef BEAM_MULTICAST_SOCKET_CHANNEL_HPP
#define BEAM_MULTICAST_SOCKET_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/MulticastSocketConnection.hpp"
#include "Beam/Network/MulticastSocketReader.hpp"
#include "Beam/Network/MulticastSocketWriter.hpp"
#include "Beam/Network/SocketIdentifier.hpp"

namespace Beam {
namespace Network {

  /** Implements the Channel interface using a multicast socket. */
  class MulticastSocketChannel {
    public:
      using Identifier = SocketIdentifier;
      using Connection = MulticastSocketConnection;
      using Reader = MulticastSocketReader;
      using Writer = MulticastSocketWriter;

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocketChannel(const IpAddress& group,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param options The options to apply to the socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocketChannel(const IpAddress& group,
        const MulticastSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param interface The interface to use.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocketChannel(const IpAddress& group, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param interface The interface to use.
       * @param options The options to apply to the socket.
       * @param socketThreadPool The thread pool used for the sockets.
       */
      MulticastSocketChannel(const IpAddress& group, const IpAddress& interface,
        const MulticastSocketOptions& options,
        Ref<SocketThreadPool> socketThreadPool);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      Identifier m_identifier;
      std::shared_ptr<MulticastSocket> m_socket;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      MulticastSocketChannel(const MulticastSocketChannel&) = delete;
      MulticastSocketChannel& operator =(
        const MulticastSocketChannel&) = delete;
  };

  inline MulticastSocketChannel::MulticastSocketChannel(const IpAddress& group,
    Ref<SocketThreadPool> socketThreadPool)
    : MulticastSocketChannel(group, MulticastSocketOptions(),
        Ref(socketThreadPool)) {}

  inline MulticastSocketChannel::MulticastSocketChannel(const IpAddress& group,
    const MulticastSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : m_identifier(group),
      m_socket(std::make_shared<MulticastSocket>(group, options,
        Ref(socketThreadPool))),
      m_connection(m_socket),
      m_reader(m_socket, group),
      m_writer(m_socket, group) {}

  inline MulticastSocketChannel::MulticastSocketChannel(const IpAddress& group,
    const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
    : MulticastSocketChannel(group, interface, MulticastSocketOptions(),
        Ref(socketThreadPool)) {}

  inline MulticastSocketChannel::MulticastSocketChannel(const IpAddress& group,
    const IpAddress& interface, const MulticastSocketOptions& options,
    Ref<SocketThreadPool> socketThreadPool)
    : m_identifier(group),
      m_socket(std::make_shared<MulticastSocket>(group, interface,
        Ref(socketThreadPool))),
      m_connection(m_socket),
      m_reader(m_socket, group),
      m_writer(m_socket, group) {}

  inline const MulticastSocketChannel::Identifier&
      MulticastSocketChannel::GetIdentifier() const {
    return m_identifier;
  }

  inline MulticastSocketChannel::Connection&
      MulticastSocketChannel::GetConnection() {
    return m_connection;
  }

  inline MulticastSocketChannel::Reader& MulticastSocketChannel::GetReader() {
    return m_reader;
  }

  inline MulticastSocketChannel::Writer& MulticastSocketChannel::GetWriter() {
    return m_writer;
  }
}

  template<>
  struct ImplementsConcept<Network::MulticastSocketChannel, IO::Channel<
    Network::MulticastSocketChannel::Identifier,
    Network::MulticastSocketChannel::Connection,
    Network::MulticastSocketChannel::Reader,
    Network::MulticastSocketChannel::Writer>> : std::true_type {};
}

#endif
