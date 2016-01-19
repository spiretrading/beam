#ifndef BEAM_MYSQLUIDDATASTOREDETAILS_HPP
#define BEAM_MYSQLUIDDATASTOREDETAILS_HPP
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Beam/UidService/UidService.hpp"

namespace Beam {
namespace UidService {
namespace Details {
namespace SqlInsert {
  sql_create_1(next_uid, 1, 0,
    mysqlpp::sql_int_unsigned, uid);
}

  sql_create_1(next_uid, 1, 0,
    mysqlpp::sql_int_unsigned, uid);

  inline bool TableExists(mysqlpp::Connection& databaseConnection,
      const std::string& schema, const char* table) {
    mysqlpp::Query query = databaseConnection.query();
    query << "SHOW TABLES IN " << schema << " LIKE " << mysqlpp::quote << table;
    mysqlpp::StoreQueryResult result = query.store();
    return !result.empty();
  }

  inline bool LoadNextUid(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "next_uid")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE next_uid ("
      "uid INTEGER UNSIGNED NOT NULL)";
    if(!query.execute()) {
      return false;
    }
    query.reset();
    SqlInsert::next_uid nextUidRow(1);
    query.insert(nextUidRow);
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadNextUid(databaseConnection, schema)) {
      return false;
    }
    return true;
  }
}
}
}

#endif
