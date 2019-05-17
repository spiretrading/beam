#ifndef BEAM_WEB_SERVICES_SQL_DEFINITIONS_HPP
#define BEAM_WEB_SERVICES_SQL_DEFINITIONS_HPP
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#include <Viper/Row.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/SessionDataStoreException.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** Represents a session stored in SQL. */
  struct SqlSession {

    /** The session's id. */
    std::string m_id;

    /** The session data. */
    IO::SharedBuffer m_session;
  };

  //! Returns a row representing the web sessions.
  inline const auto& GetWebSessionsRow() {
    static auto ROW = Viper::Row<SqlSession>().
      add_column("id", Viper::varchar(64), &SqlSession::m_id).
      add_column("session", &SqlSession::m_session).
      set_primary_key("id");
    return ROW;
  }

  template<typename Session, typename Receiver>
  auto FromRow(const SqlSession& sqlSession, Receiver& receiver) {
    receiver.SetSource(Ref(sqlSession.m_session));
    auto session = std::make_unique<Session>();
    try {
      receiver.Shuttle(*session);
    } catch(const Serialization::SerializationException&) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException{
        "Unable to load session."});
    }
    return session;
  }

  template<typename Session, typename Sender>
  auto ToRow(const Session& session, Sender& sender) {
    auto buffer = typename Sender::Buffer();
    sender.SetSink(Ref(buffer));
    try {
      sender.Shuttle(session);
    } catch(const Serialization::SerializationException&) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException(
        "Unable to store session."));
    }
    return SqlSession{session.GetId(), std::move(buffer)};
  }
}

#endif
