#ifndef BEAM_NULL_CHANNEL_HPP
#define BEAM_NULL_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/NullReader.hpp"
#include "Beam/IO/NullWriter.hpp"

namespace Beam {

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
      explicit NullChannel(NamedChannelIdentifier identifier) noexcept;

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      NamedChannelIdentifier m_identifier;
      NullConnection m_connection;
      NullReader m_reader;
      NullWriter m_writer;

      NullChannel(const NullChannel&) = delete;
      NullChannel& operator =(const NullChannel&) = delete;
  };

  inline NullChannel::NullChannel(NamedChannelIdentifier identifier) noexcept
    : m_identifier(std::move(identifier)) {}

  inline const NullChannel::Identifier& NullChannel::get_identifier() const {
    return m_identifier;
  }

  inline NullChannel::Connection& NullChannel::get_connection() {
    return m_connection;
  }

  inline NullChannel::Reader& NullChannel::get_reader() {
    return m_reader;
  }

  inline NullChannel::Writer& NullChannel::get_writer() {
    return m_writer;
  }
}

#endif
