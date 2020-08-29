#ifndef BEAM_LOCAL_SERVER_CHANNEL_HPP
#define BEAM_LOCAL_SERVER_CHANNEL_HPP
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"

namespace Beam {
namespace IO {

  /**
   * Implements the server side of a local Channel.
   * @param <B> The type of Buffer to use.
   */
  template<typename B>
  class LocalServerChannel {
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
       * Constructs a LocalServerChannel.
       * @param name The name of the Channel.
       * @param reader The Reader to use.
       * @param writer The Writer to use.
       * @param clientWriter The client-side Writer.
       */
      LocalServerChannel(std::string name, std::unique_ptr<Reader> reader,
        std::shared_ptr<Writer> writer, std::shared_ptr<Writer> clientWriter);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      Identifier m_identifier;
      std::unique_ptr<Reader> m_reader;
      std::shared_ptr<Writer> m_writer;
      Connection m_connection;

      LocalServerChannel(const LocalServerChannel&) = delete;
      LocalServerChannel& operator =(const LocalServerChannel&) = delete;
  };

  template<typename B>
  LocalServerChannel<B>::LocalServerChannel(std::string name,
    std::unique_ptr<Reader> reader, std::shared_ptr<Writer> writer,
    std::shared_ptr<Writer> clientWriter)
    : m_identifier(std::move(name)),
      m_reader(std::move(reader)),
      m_writer(writer),
      m_connection(std::move(writer), std::move(clientWriter)) {}

  template<typename B>
  const typename LocalServerChannel<B>::Identifier&
      LocalServerChannel<B>::GetIdentifier() const {
    return m_identifier;
  }

  template<typename B>
  typename LocalServerChannel<B>::Connection&
      LocalServerChannel<B>::GetConnection() {
    return m_connection;
  }

  template<typename B>
  typename LocalServerChannel<B>::Reader& LocalServerChannel<B>::GetReader() {
    return *m_reader;
  }

  template<typename B>
  typename LocalServerChannel<B>::Writer& LocalServerChannel<B>::GetWriter() {
    return *m_writer;
  }
}

  template<typename B>
  struct ImplementsConcept<IO::LocalServerChannel<B>,
    IO::Channel<typename IO::LocalServerChannel<B>::Identifier,
    typename IO::LocalServerChannel<B>::Connection,
    typename IO::LocalServerChannel<B>::Reader,
    typename IO::LocalServerChannel<B>::Writer>> : std::true_type {};
}

#endif
