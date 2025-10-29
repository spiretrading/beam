#ifndef BEAM_MULTICAST_SOCKET_CHANNEL_HPP
#define BEAM_MULTICAST_SOCKET_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/MulticastSocketConnection.hpp"
#include "Beam/Network/MulticastSocketReader.hpp"
#include "Beam/Network/MulticastSocketWriter.hpp"
#include "Beam/Network/SocketIdentifier.hpp"

namespace Beam {

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
       */
      explicit MulticastSocketChannel(const IpAddress& group);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param options The options to apply to the socket.
       */
      MulticastSocketChannel(
        const IpAddress& group, const MulticastSocketOptions& options);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param interface The interface to use.
       */
      MulticastSocketChannel(
        const IpAddress& group, const IpAddress& interface);

      /**
       * Constructs a MulticastSocketChannel.
       * @param group The group to join.
       * @param interface The interface to use.
       * @param options The options to apply to the socket.
       */
      MulticastSocketChannel(const IpAddress& group, const IpAddress& interface,
        const MulticastSocketOptions& options);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

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

  inline MulticastSocketChannel::MulticastSocketChannel(const IpAddress& group)
    : MulticastSocketChannel(group, MulticastSocketOptions()) {}

  inline MulticastSocketChannel::MulticastSocketChannel(
    const IpAddress& group, const MulticastSocketOptions& options)
    : m_identifier(group),
      m_socket(std::make_shared<MulticastSocket>(group, options)),
      m_connection(m_socket),
      m_reader(m_socket, group),
      m_writer(m_socket, group) {}

  inline MulticastSocketChannel::MulticastSocketChannel(
    const IpAddress& group, const IpAddress& interface)
    : MulticastSocketChannel(group, interface, MulticastSocketOptions()) {}

  inline MulticastSocketChannel::MulticastSocketChannel(const IpAddress& group,
    const IpAddress& interface, const MulticastSocketOptions& options)
    : m_identifier(group),
      m_socket(std::make_shared<MulticastSocket>(group, interface, options)),
      m_connection(m_socket),
      m_reader(m_socket, group),
      m_writer(m_socket, group) {}

  inline const MulticastSocketChannel::Identifier&
      MulticastSocketChannel::get_identifier() const {
    return m_identifier;
  }

  inline MulticastSocketChannel::Connection&
      MulticastSocketChannel::get_connection() {
    return m_connection;
  }

  inline MulticastSocketChannel::Reader& MulticastSocketChannel::get_reader() {
    return m_reader;
  }

  inline MulticastSocketChannel::Writer& MulticastSocketChannel::get_writer() {
    return m_writer;
  }
}

#endif
