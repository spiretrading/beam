#ifndef BEAM_LOCAL_SERVER_CHANNEL_HPP
#define BEAM_LOCAL_SERVER_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

namespace Beam {

  /** Implements the server side of a local Channel. */
  class LocalServerChannel {
    public:
      using Identifier = NamedChannelIdentifier;
      using Connection = LocalConnection;
      using Reader = PipedReader;
      using Writer = PipedWriter;

      /**
       * Constructs a LocalServerChannel.
       * @param name The name of the Channel.
       * @param reader The Reader to use.
       * @param writer The Writer to use.
       * @param client_writer The client-side Writer.
       */
      LocalServerChannel(std::string name, std::unique_ptr<Reader> reader,
        std::shared_ptr<Writer> writer, std::shared_ptr<Writer> clientWriter);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      Identifier m_identifier;
      std::unique_ptr<Reader> m_reader;
      std::shared_ptr<Writer> m_writer;
      Connection m_connection;

      LocalServerChannel(const LocalServerChannel&) = delete;
      LocalServerChannel& operator =(const LocalServerChannel&) = delete;
  };

  inline LocalServerChannel::LocalServerChannel(std::string name,
    std::unique_ptr<Reader> reader, std::shared_ptr<Writer> writer,
    std::shared_ptr<Writer> client_writer)
    : m_identifier(std::move(name)),
      m_reader(std::move(reader)),
      m_writer(writer),
      m_connection(std::move(writer), std::move(client_writer)) {}

  inline const LocalServerChannel::Identifier&
      LocalServerChannel::get_identifier() const {
    return m_identifier;
  }

  inline LocalServerChannel::Connection& LocalServerChannel::get_connection() {
    return m_connection;
  }

  inline LocalServerChannel::Reader& LocalServerChannel::get_reader() {
    return *m_reader;
  }

  inline LocalServerChannel::Writer& LocalServerChannel::get_writer() {
    return *m_writer;
  }
}

#endif
