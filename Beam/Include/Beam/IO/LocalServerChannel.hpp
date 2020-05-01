#ifndef BEAM_LOCALSERVERCHANNEL_HPP
#define BEAM_LOCALSERVERCHANNEL_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {

  /*! \class LocalServerChannel
      \brief Implements the server side of a local Channel.
      \tparam BufferType The type of Buffer to use.
   */
  template<typename BufferType>
  class LocalServerChannel : private boost::noncopyable {
    public:

      //! The type of Buffer to use.
      using Buffer = BufferType;

      //! The type of LocalServerConnection this connects to.
      using LocalServerConnection = IO::LocalServerConnection<Buffer>;

      using Identifier = NamedChannelIdentifier;
      using Connection = LocalConnection<Buffer>;
      using Reader = PipedReader<Buffer>;
      using Writer = PipedWriter<Buffer>;

      //! Constructs a LocalServerChannel.
      /*!
        \param name The name of the Channel.
        \param serverConnection The server to connect to.
        \param clientChannel The client side of the Channel.
      */
      LocalServerChannel(const std::string& name,
        Ref<LocalServerConnection> serverConnection,
        Ref<LocalClientChannel<Buffer>> clientChannel);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      friend class IO::LocalClientChannel<Buffer>;
      friend class IO::LocalServerConnection<Buffer>;
      Identifier m_identifier;
      Reader m_reader;
      std::shared_ptr<Writer> m_writer;
      Connection m_connection;
  };

  template<typename BufferType>
  LocalServerChannel<BufferType>::LocalServerChannel(const std::string& name,
      Ref<LocalServerConnection> serverConnection,
      Ref<LocalClientChannel<Buffer>> clientChannel)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_identifier("server@" + name),
        m_writer(std::make_shared<Writer>(Ref(clientChannel->GetReader()))),
        m_connection(serverConnection->m_pendingChannels, m_writer) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename BufferType>
  const typename LocalServerChannel<BufferType>::Identifier&
      LocalServerChannel<BufferType>::GetIdentifier() const {
    return m_identifier;
  }

  template<typename BufferType>
  typename LocalServerChannel<BufferType>::Connection&
      LocalServerChannel<BufferType>::GetConnection() {
    return m_connection;
  }

  template<typename BufferType>
  typename LocalServerChannel<BufferType>::Reader&
      LocalServerChannel<BufferType>::GetReader() {
    return m_reader;
  }

  template<typename BufferType>
  typename LocalServerChannel<BufferType>::Writer&
      LocalServerChannel<BufferType>::GetWriter() {
    return *m_writer;
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::LocalServerChannel<BufferType>,
    IO::Channel<typename IO::LocalServerChannel<BufferType>::Identifier,
    typename IO::LocalServerChannel<BufferType>::Connection,
    typename IO::LocalServerChannel<BufferType>::Reader,
    typename IO::LocalServerChannel<BufferType>::Writer>> : std::true_type {};
}

#endif
