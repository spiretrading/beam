#ifndef BEAM_LOCAL_CLIENT_CHANNEL_HPP
#define BEAM_LOCAL_CLIENT_CHANNEL_HPP
#include <optional>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"

namespace Beam {
namespace IO {
namespace Details {
  template<typename Buffer>
  void Connect(LocalServerConnection<Buffer>&, LocalClientChannel<Buffer>&,
    const std::string&);
}

  /**
   * Implements the client side of a local Channel.
   * @param <B> The type of Buffer to use.
   */
  template<typename B>
  class LocalClientChannel {
    public:

      /** The type of Buffer to use. */
      using Buffer = B;

      /** The type of LocalServerConnection this connects to. */
      using LocalServerConnection = IO::LocalServerConnection<Buffer>;

      using Identifier = NamedChannelIdentifier;
      using Connection = LocalConnection<Buffer>;
      using Reader = PipedReader<Buffer>;
      using Writer = PipedWriter<Buffer>;

      /**
       * Constructs a LocalClientChannel.
       * @param name The name of the Channel.
       * @param server The server to connect to.
       */
      LocalClientChannel(const std::string& name,
        LocalServerConnection& server);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      friend LocalServerConnection;
      Identifier m_identifier;
      std::unique_ptr<Reader> m_reader;
      std::shared_ptr<Writer> m_writer;
      std::optional<Connection> m_connection;

      LocalClientChannel(const LocalClientChannel&) = delete;
      LocalClientChannel& operator =(const LocalClientChannel&) = delete;
  };

  template<typename B>
  LocalClientChannel<B>::LocalClientChannel(const std::string& name,
      LocalServerConnection& server)
      : m_identifier("client@" + name) {
    Details::Connect(server, *this, name);
  }

  template<typename B>
  const typename LocalClientChannel<B>::Identifier&
      LocalClientChannel<B>::GetIdentifier() const {
    return m_identifier;
  }

  template<typename B>
  typename LocalClientChannel<B>::Connection&
      LocalClientChannel<B>::GetConnection() {
    return *m_connection;
  }

  template<typename B>
  typename LocalClientChannel<B>::Reader& LocalClientChannel<B>::GetReader() {
    return *m_reader;
  }

  template<typename B>
  typename LocalClientChannel<B>::Writer& LocalClientChannel<B>::GetWriter() {
    return *m_writer;
  }
}

  template<typename B>
  struct ImplementsConcept<IO::LocalClientChannel<B>,
    IO::Channel<typename IO::LocalClientChannel<B>::Identifier,
    typename IO::LocalClientChannel<B>::Connection,
    typename IO::LocalClientChannel<B>::Reader,
    typename IO::LocalClientChannel<B>::Writer>> : std::true_type {};
}

#endif
