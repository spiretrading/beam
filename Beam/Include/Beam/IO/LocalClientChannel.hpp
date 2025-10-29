#ifndef BEAM_LOCAL_CLIENT_CHANNEL_HPP
#define BEAM_LOCAL_CLIENT_CHANNEL_HPP
#include <boost/optional/optional.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"

namespace Beam {
  class LocalServerConnection;
  class LocalClientChannel;

namespace Details {
  void connect(LocalServerConnection&, LocalClientChannel&, const std::string&);
}

  /** Implements the client side of a local Channel. */
  class LocalClientChannel {
    public:
      using Identifier = NamedChannelIdentifier;
      using Connection = LocalConnection;
      using Reader = PipedReader;
      using Writer = PipedWriter;

      /**
       * Constructs a LocalClientChannel.
       * @param name The name of the Channel.
       * @param server The server to connect to.
       */
      LocalClientChannel(
        const std::string& name, LocalServerConnection& server);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      friend LocalServerConnection;
      Identifier m_identifier;
      std::unique_ptr<Reader> m_reader;
      std::shared_ptr<Writer> m_writer;
      boost::optional<Connection> m_connection;

      LocalClientChannel(const LocalClientChannel&) = delete;
      LocalClientChannel& operator =(const LocalClientChannel&) = delete;
  };

  inline LocalClientChannel::LocalClientChannel(
      const std::string& name, LocalServerConnection& server)
      : m_identifier("client@" + name) {
    Details::connect(server, *this, name);
  }

  inline const LocalClientChannel::Identifier&
      LocalClientChannel::get_identifier() const {
    return m_identifier;
  }

  inline LocalClientChannel::Connection& LocalClientChannel::get_connection() {
    return *m_connection;
  }

  inline LocalClientChannel::Reader& LocalClientChannel::get_reader() {
    return *m_reader;
  }

  inline LocalClientChannel::Writer& LocalClientChannel::get_writer() {
    return *m_writer;
  }
}

#endif
