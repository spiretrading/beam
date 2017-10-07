#ifndef BEAM_MYSQL_SESSION_DATA_STORE_DETAILS_HPP
#define BEAM_MYSQL_SESSION_DATA_STORE_DETAILS_HPP
#include <boost/throw_exception.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/MySql/Utilities.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/SessionDataStoreException.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {
namespace Details {
  sql_create_2(web_sessions, 2, 0,
    mysqlpp::sql_varchar, id,
    mysqlpp::sql_blob, session);

  inline bool LoadWebSessionsTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(MySql::TestTable(schema, "web_sessions", databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE web_sessions ("
      "id VARCHAR (64) BINARY NOT NULL,"
      "session BLOB NOT NULL)";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadWebSessionsTable(databaseConnection, schema)) {
      return false;
    }
    return true;
  }

  template<typename SessionType>
  inline std::unique_ptr<SessionType> FromRow(const web_sessions& row,
      Serialization::JsonReceiver<IO::SharedBuffer>& receiver) {
    IO::SharedBuffer buffer{row.session.data(), row.session.size()};
    receiver.SetSource(Ref(buffer));
    auto session = std::make_unique<SessionType>();
    try {
      receiver.Shuttle(*session);
    } catch(const Serialization::SerializationException&) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException{
        "Unable to load session."});
    }
    return session;
  }

  template<typename SessionType>
  inline web_sessions ToRow(const SessionType& session,
      Serialization::JsonSender<IO::SharedBuffer>& sender) {
    IO::SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    try {
      sender.Shuttle(session);
    } catch(const Serialization::SerializationException&) {
      BOOST_THROW_EXCEPTION(SessionDataStoreException{
        "Unable to store session."});
    }
    Details::web_sessions row{session.GetId(),
      mysqlpp::sql_blob{buffer.GetData(), buffer.GetSize()}};
    return row;
  }
}
}
}

#endif
