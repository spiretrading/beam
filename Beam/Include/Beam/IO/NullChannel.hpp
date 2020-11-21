#ifndef BEAM_NULL_CHANNEL_HPP
#define BEAM_NULL_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/NullReader.hpp"
#include "Beam/IO/NullWriter.hpp"

namespace Beam {
namespace IO {

  /** Composes a NullConnection, NullReader and NullWriter into a Channel. */
  class NullChannel {
    public:
      using Identifier = NamedChannelIdentifier;
      using Connection = NullConnection;
      using Reader = NullReader;
      using Writer = NullWriter;

      /** Constructs a default NullChannel. */
      NullChannel() = default;

      /**
       * Constructs a NullChannel with an identifier.
       * @param identifier The identifier to use.
       */
      NullChannel(const NamedChannelIdentifier& identifier);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      NamedChannelIdentifier m_identifier;
      NullConnection m_connection;
      NullReader m_reader;
      NullWriter m_writer;

      NullChannel(const NullChannel&) = delete;
      NullChannel& operator =(const NullChannel&) = delete;
  };

  inline NullChannel::NullChannel(const NamedChannelIdentifier& identifier)
    : m_identifier(identifier) {}

  inline const NullChannel::Identifier& NullChannel::GetIdentifier() const {
    return m_identifier;
  }

  inline NullChannel::Connection& NullChannel::GetConnection() {
    return m_connection;
  }

  inline NullChannel::Reader& NullChannel::GetReader() {
    return m_reader;
  }

  inline NullChannel::Writer& NullChannel::GetWriter() {
    return m_writer;
  }
}

  template<>
  struct ImplementsConcept<IO::NullChannel,
    IO::Channel<IO::NamedChannelIdentifier, IO::NullConnection, IO::NullReader,
      IO::NullWriter>> : std::true_type {};
}

#endif
