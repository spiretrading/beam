#ifndef BEAM_SECURE_SOCKET_CHANNEL_HPP
#define BEAM_SECURE_SOCKET_CHANNEL_HPP
#include <boost/asio/ip/tcp.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SecureSocketConnection.hpp"
#include "Beam/Network/SecureSocketOptions.hpp"
#include "Beam/Network/SecureSocketReader.hpp"
#include "Beam/Network/SecureSocketWriter.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"

namespace Beam {

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
       */
      explicit SecureSocketChannel(const IpAddress& address);

      /**
       * Constructs a SecureSocketChannel.
       * @param address The IP address to connect to.
       * @param options The options to apply to the socket.
       */
      SecureSocketChannel(
        const IpAddress& address, const SecureSocketOptions& options);

      /**
       * Constructs a SecureSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       */
      SecureSocketChannel(const IpAddress& address, const IpAddress& interface);

      /**
       * Constructs a SecureSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       * @param options The options to apply to the socket.
       */
      SecureSocketChannel(const IpAddress& address, const IpAddress& interface,
        const SecureSocketOptions& options);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       */
      explicit SecureSocketChannel(const std::vector<IpAddress>& addresses);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param options The options to apply to the socket.
       */
      SecureSocketChannel(const std::vector<IpAddress>& addresses,
        const SecureSocketOptions& options);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       */
      SecureSocketChannel(
        const std::vector<IpAddress>& addresses, const IpAddress& interface);

      /**
       * Constructs a SecureSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       * @param options The options to apply to the socket.
       */
      SecureSocketChannel(const std::vector<IpAddress>& addresses,
        const IpAddress& interface, const SecureSocketOptions& options);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      friend class SecureServerSocket;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;
      Identifier m_identifier;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      SecureSocketChannel();
      SecureSocketChannel(const SecureSocketChannel&) = delete;
      SecureSocketChannel& operator =(const SecureSocketChannel&) = delete;
      void set_address(const IpAddress& address);
  };

  inline SecureSocketChannel::SecureSocketChannel(const IpAddress& address)
    : SecureSocketChannel(address, SecureSocketOptions()) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const IpAddress& address, const SecureSocketOptions& options)
    : SecureSocketChannel(std::vector{address}, options) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const IpAddress& address, const IpAddress& interface)
    : SecureSocketChannel(address, interface, SecureSocketOptions()) {}

  inline SecureSocketChannel::SecureSocketChannel(const IpAddress& address,
    const IpAddress& interface, const SecureSocketOptions& options)
    : SecureSocketChannel(std::vector{address}, interface, options) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses)
    : SecureSocketChannel(addresses, SecureSocketOptions()) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses, const SecureSocketOptions& options)
    : m_socket(std::make_shared<Details::SecureSocketEntry>(
        ServiceThreadPool::get().get_context(),
        ServiceThreadPool::get().get_context())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface)
    : SecureSocketChannel(addresses, interface, SecureSocketOptions()) {}

  inline SecureSocketChannel::SecureSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface,
    const SecureSocketOptions& options)
    : m_socket(std::make_shared<Details::SecureSocketEntry>(
        ServiceThreadPool::get().get_context(),
        ServiceThreadPool::get().get_context())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses, interface),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline const SecureSocketChannel::Identifier&
      SecureSocketChannel::get_identifier() const {
    return m_identifier;
  }

  inline SecureSocketChannel::Connection&
      SecureSocketChannel::get_connection() {
    return m_connection;
  }

  inline SecureSocketChannel::Reader& SecureSocketChannel::get_reader() {
    return m_reader;
  }

  inline SecureSocketChannel::Writer& SecureSocketChannel::get_writer() {
    return m_writer;
  }

  inline SecureSocketChannel::SecureSocketChannel()
    : m_socket(std::make_shared<Details::SecureSocketEntry>(
        ServiceThreadPool::get().get_context(),
        ServiceThreadPool::get().get_context())),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline void SecureSocketChannel::set_address(const IpAddress& address) {
    m_identifier = SocketIdentifier(address);
  }
}

#endif
