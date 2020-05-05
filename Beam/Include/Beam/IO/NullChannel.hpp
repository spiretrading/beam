#ifndef BEAM_NULLCHANNEL_HPP
#define BEAM_NULLCHANNEL_HPP
#include <string>
#include <boost/noncopyable.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/NullReader.hpp"
#include "Beam/IO/NullWriter.hpp"

namespace Beam {
namespace IO {

  /*  \class NullChannel
      \brief Composes a NullConnection, NullReader and NullWriter into a
             Channel.
   */
  class NullChannel : private boost::noncopyable {
    public:
      using Identifier = std::string;
      using Connection = NullConnection;
      using Reader = NullReader;
      using Writer = NullWriter;

      //! Constructs a default NullChannel.
      NullChannel() = default;

      //! Constructs a NullChannel with an identifier.
      /*!
        \param identifier The identifier to use.
      */
      NullChannel(const std::string& identifier);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      std::string m_identifier;
      NullConnection m_connection;
      NullReader m_reader;
      NullWriter m_writer;
  };

  inline NullChannel::NullChannel(const std::string& identifier)
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
}

#endif
