#ifndef BEAM_MULTICASTSOCKETCHANNEL_HPP
#define BEAM_MULTICASTSOCKETCHANNEL_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/MulticastSocketConnection.hpp"
#include "Beam/Network/MulticastSocketReader.hpp"
#include "Beam/Network/MulticastSocketWriter.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/SocketIdentifier.hpp"

namespace Beam {
namespace Network {

  /*! \class MulticastSocketChannel
      \brief Implements the Channel interface using a UDP multicast socket.
   */
  class MulticastSocketChannel : private boost::noncopyable {
    public:
      using Identifier = SocketIdentifier;
      using Connection = MulticastSocketConnection;
      using Reader = MulticastSocketReader;
      using Writer = MulticastSocketWriter;

      //! Constructs a MulticastSocketChannel.
      /*!
        \param address The address to open.
        \param interface The interface to listen on.
        \param socketThreadPool The thread pool used for the sockets.
      */
      MulticastSocketChannel(const IpAddress& address,
        const IpAddress& interface, Ref<SocketThreadPool> socketThreadPool);

      //! Returns the underlying MulticastSocket.
      MulticastSocket& GetSocket();

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
  };

  inline MulticastSocketChannel::MulticastSocketChannel(
      const IpAddress& address, const IpAddress& interface,
      Ref<SocketThreadPool> socketThreadPool)
      : m_identifier(address),
        m_socket(std::make_shared<MulticastSocket>(address, interface,
          Ref(socketThreadPool))),
        m_connection(m_socket),
        m_reader(m_socket, address),
        m_writer(m_socket, address) {}

  inline MulticastSocket& MulticastSocketChannel::GetSocket() {
    return *m_socket;
  }

  inline const MulticastSocketChannel::Identifier& MulticastSocketChannel::
      GetIdentifier() const {
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
    Network::MulticastSocketChannel::Writer>> :
    std::true_type {};
}

#endif
