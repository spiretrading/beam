#ifndef BEAM_TCP_SOCKET_CHANNEL_HPP
#define BEAM_TCP_SOCKET_CHANNEL_HPP
#include <boost/asio/ip/tcp.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/TcpSocketConnection.hpp"
#include "Beam/Network/TcpSocketOptions.hpp"
#include "Beam/Network/TcpSocketReader.hpp"
#include "Beam/Network/TcpSocketWriter.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"

namespace Beam {

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
       */
      explicit TcpSocketChannel(const IpAddress& address);

      /**
       * Constructs a TcpSocketChannel.
       * @param address The IP address to connect to.
       * @param options The set of TcpSocketOptions to apply.
       */
      TcpSocketChannel(
        const IpAddress& address, const TcpSocketOptions& options);

      /**
       * Constructs a TcpSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       */
      TcpSocketChannel(const IpAddress& address, const IpAddress& interface);

      /**
       * Constructs a TcpSocketChannel.
       * @param address The IP address to connect to.
       * @param interface The interface to bind to.
       * @param options The set of TcpSocketOptions to apply.
       */
      TcpSocketChannel(const IpAddress& address, const IpAddress& interface,
        const TcpSocketOptions& options);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       */
      explicit TcpSocketChannel(const std::vector<IpAddress>& addresses);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param options The set of TcpSocketOptions to apply.
       */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        const TcpSocketOptions& options);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       */
      TcpSocketChannel(
        const std::vector<IpAddress>& addresses, const IpAddress& interface);

      /**
       * Constructs a TcpSocketChannel.
       * @param addresses The list of IP addresses to try to connect to.
       * @param interface The interface to bind to.
       * @param options The set of TcpSocketOptions to apply.
       */
      TcpSocketChannel(const std::vector<IpAddress>& addresses,
        const IpAddress& interface, const TcpSocketOptions& options);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      friend class TcpServerSocket;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;
      Identifier m_identifier;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      TcpSocketChannel();
      TcpSocketChannel(const TcpSocketChannel&) = delete;
      TcpSocketChannel& operator =(const TcpSocketChannel&) = delete;
      void set(const IpAddress& address);
  };

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address)
    : TcpSocketChannel(address, TcpSocketOptions()) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const IpAddress& address, const TcpSocketOptions& options)
    : TcpSocketChannel(std::vector{address}, options) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const IpAddress& address, const IpAddress& interface)
    : TcpSocketChannel(address, interface, TcpSocketOptions()) {}

  inline TcpSocketChannel::TcpSocketChannel(const IpAddress& address,
    const IpAddress& interface, const TcpSocketOptions& options)
    : TcpSocketChannel(std::vector{address}, interface, options) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses)
    : TcpSocketChannel(addresses, TcpSocketOptions()) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses, const TcpSocketOptions& options)
    : m_socket(std::make_shared<Details::TcpSocketEntry>(
        ServiceThreadPool::get().get_context(),
        ServiceThreadPool::get().get_context())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface)
    : TcpSocketChannel(addresses, interface, TcpSocketOptions()) {}

  inline TcpSocketChannel::TcpSocketChannel(
    const std::vector<IpAddress>& addresses, const IpAddress& interface,
    const TcpSocketOptions& options)
    : m_socket(std::make_shared<Details::TcpSocketEntry>(
        ServiceThreadPool::get().get_context(),
        ServiceThreadPool::get().get_context())),
      m_identifier(addresses.front()),
      m_connection(m_socket, options, addresses, interface),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline const TcpSocketChannel::Identifier&
      TcpSocketChannel::get_identifier() const {
    return m_identifier;
  }

  inline TcpSocketChannel::Connection& TcpSocketChannel::get_connection() {
    return m_connection;
  }

  inline TcpSocketChannel::Reader& TcpSocketChannel::get_reader() {
    return m_reader;
  }

  inline TcpSocketChannel::Writer& TcpSocketChannel::get_writer() {
    return m_writer;
  }

  inline TcpSocketChannel::TcpSocketChannel()
    : m_socket(std::make_shared<Details::TcpSocketEntry>(
        ServiceThreadPool::get().get_context(),
        ServiceThreadPool::get().get_context())),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline void TcpSocketChannel::set(const IpAddress& address) {
    m_identifier = SocketIdentifier(address);
  }
}

#endif
