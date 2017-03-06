#ifndef BEAM_LOCALCLIENTCHANNEL_HPP
#define BEAM_LOCALCLIENTCHANNEL_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/LocalServerChannel.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace IO {

  /*! \class LocalClientChannel
      \brief Implements the client side of a local Channel.
      \tparam BufferType The type of Buffer to use.
   */
  template<typename BufferType>
  class LocalClientChannel : private boost::noncopyable {
    public:

      //! The type of Buffer to use.
      using Buffer = BufferType;

      //! The type of LocalServerConnection this connects to.
      using LocalServerConnection = IO::LocalServerConnection<Buffer>;

      using Identifier = NamedChannelIdentifier;
      using Connection = LocalConnection<Buffer>;
      using Reader = PipedReader<Buffer>;
      using Writer = PipedWriter<Buffer>;

      //! Constructs a LocalClientChannel.
      /*!
        \param name The name of the Channel.
        \param server The server to connect to.
      */
      LocalClientChannel(const std::string& name,
        RefType<LocalServerConnection> server);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      Identifier m_identifier;
      Reader m_reader;
      std::shared_ptr<Writer> m_writer;
      DelayPtr<Connection> m_connection;
  };

  template<typename BufferType>
  LocalClientChannel<BufferType>::LocalClientChannel(const std::string& name,
      RefType<LocalServerConnection> server)
      : m_identifier("client@" + name) {
    auto serverChannel =  std::make_unique<LocalServerChannel<Buffer>>(name,
      Ref(server), Ref(*this));
    m_writer = std::make_shared<Writer>(Ref(serverChannel->GetReader()));
    m_connection.Initialize(server->m_pendingChannels, m_writer);
    m_connection->m_channel = this;
    m_connection->m_endpointWriter = serverChannel->m_writer;
    serverChannel->m_connection.m_endpointWriter = m_writer;
    server->AddChannel(Ref(*this), std::move(serverChannel));
  }

  template<typename BufferType>
  const typename LocalClientChannel<BufferType>::Identifier&
      LocalClientChannel<BufferType>::GetIdentifier() const {
    return m_identifier;
  }

  template<typename BufferType>
  typename LocalClientChannel<BufferType>::Connection&
      LocalClientChannel<BufferType>::GetConnection() {
    return *m_connection;
  }

  template<typename BufferType>
  typename LocalClientChannel<BufferType>::Reader&
      LocalClientChannel<BufferType>::GetReader() {
    return m_reader;
  }

  template<typename BufferType>
  typename LocalClientChannel<BufferType>::Writer&
      LocalClientChannel<BufferType>::GetWriter() {
    return *m_writer;
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::LocalClientChannel<BufferType>,
    IO::Channel<typename IO::LocalClientChannel<BufferType>::Identifier,
    typename IO::LocalClientChannel<BufferType>::Connection,
    typename IO::LocalClientChannel<BufferType>::Reader,
    typename IO::LocalClientChannel<BufferType>::Writer>> : std::true_type {};
}

#endif
