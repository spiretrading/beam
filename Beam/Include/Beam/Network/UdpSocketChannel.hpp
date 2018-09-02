#ifndef BEAM_UDPSOCKETCHANNEL_HPP
#define BEAM_UDPSOCKETCHANNEL_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketConnection.hpp"
#include "Beam/Network/UdpSocketReader.hpp"
#include "Beam/Network/UdpSocketWriter.hpp"

namespace Beam {
namespace Network {

  /*! \class UdpSocketChannel
      \brief Implements the Channel interface using a UDP socket.
   */
  class UdpSocketChannel : private boost::noncopyable {
    public:
      using Identifier = SocketIdentifier;
      using Connection = UdpSocketConnection;
      using Reader = UdpSocketReader;
      using Writer = UdpSocketWriter;

      //! Constructs a UdpSocketChannel.
      /*!
        \param address The address to open.
        \param socketThreadPool The thread pool used for the sockets.
      */
      UdpSocketChannel(const IpAddress& address,
        Ref<SocketThreadPool> socketThreadPool);

      //! Constructs a UdpSocketChannel.
      /*!
        \param address The address to open.
        \param interface The interface to use.
        \param socketThreadPool The thread pool used for the sockets.
      */
      UdpSocketChannel(const IpAddress& address, const IpAddress& interface,
        Ref<SocketThreadPool> socketThreadPool);

      //! Returns the underlying UdpSocket.
      UdpSocket& GetSocket();

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      Identifier m_identifier;
      std::shared_ptr<UdpSocket> m_socket;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;
  };

  inline UdpSocketChannel::UdpSocketChannel(const IpAddress& address,
      Ref<SocketThreadPool> socketThreadPool)
      : m_identifier(address),
        m_socket(std::make_shared<UdpSocket>(address, Ref(socketThreadPool))),
        m_connection(m_socket),
        m_reader(m_socket),
        m_writer(m_socket) {}

  inline UdpSocketChannel::UdpSocketChannel(const IpAddress& address,
      const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool)
      : m_identifier(address),
        m_socket(std::make_shared<UdpSocket>(address, interface,
          Ref(socketThreadPool))),
        m_connection(m_socket),
        m_reader(m_socket),
        m_writer(m_socket) {}

  inline UdpSocket& UdpSocketChannel::GetSocket() {
    return *m_socket;
  }

  inline const UdpSocketChannel::Identifier& UdpSocketChannel::
      GetIdentifier() const {
    return m_identifier;
  }

  inline UdpSocketChannel::Connection& UdpSocketChannel::GetConnection() {
    return m_connection;
  }

  inline UdpSocketChannel::Reader& UdpSocketChannel::GetReader() {
    return m_reader;
  }

  inline UdpSocketChannel::Writer& UdpSocketChannel::GetWriter() {
    return m_writer;
  }
}

  template<>
  struct ImplementsConcept<Network::UdpSocketChannel, IO::Channel<
    Network::UdpSocketChannel::Identifier,
    Network::UdpSocketChannel::Connection,
    Network::UdpSocketChannel::Reader, Network::UdpSocketChannel::Writer>> :
    std::true_type {};
}

#endif
