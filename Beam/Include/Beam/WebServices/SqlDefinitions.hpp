#ifndef BEAM_WEB_SERVICES_SQL_DEFINITIONS_HPP
#define BEAM_WEB_SERVICES_SQL_DEFINITIONS_HPP
#include <string>
#include <boost/throw_exception.hpp>
#include <Viper/Row.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Sql/Conversions.hpp"
#include "Beam/WebServices/WebSessionDataStoreException.hpp"

namespace Beam {

  /** Represents a session stored in SQL. */
  struct SqlSession {

    /** The session's id. */
    std::string m_id;

    /** The session data. */
    SharedBuffer m_session;
  };

  /** Returns a row representing the web sessions. */
  inline const auto& get_web_sessions_row() {
    static auto ROW = Viper::Row<SqlSession>().
      add_column("id", Viper::varchar(64), &SqlSession::m_id).
      add_column("session", &SqlSession::m_session).
      set_primary_key("id");
    return ROW;
  }

  template<typename Session, IsReceiver R>
  auto from_row(const SqlSession& sql_session, R& receiver) {
    receiver.set(Ref(sql_session.m_session));
    auto session = std::make_unique<Session>();
    try {
      receiver.receive(*session);
    } catch(const SerializationException&) {
      boost::throw_with_location(
        WebSessionDataStoreException("Unable to load session."));
    }
    return session;
  }

  template<typename Session, IsSender S>
  auto to_row(const Session& session, S& sender) {
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    try {
      sender.send(session);
    } catch(const SerializationException&) {
      boost::throw_with_location(
        WebSessionDataStoreException("Unable to store session."));
    }
    return SqlSession(session.get_id(), std::move(buffer));
  }
}

#endif
