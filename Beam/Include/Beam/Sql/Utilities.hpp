#ifndef BEAM_SQL_UTILITIES_HPP
#define BEAM_SQL_UTILITIES_HPP
#include <string>
#include <mysql++/connection.h>
#include <mysql++/query.h>
#include "Beam/Sql/Sql.hpp"

namespace Beam {

  //! Tests if a table is a member of a schema.
  /*!
    \param schema The schema to test.
    \param table The table to check for.
    \param connection The MySQL connection used to perform the test.
    \return <code>true</code> iff the <i>table</i> is a part of the
            <i>schema</i>.
  */
  inline bool TestTable(const std::string& schema, const std::string& table,
      mysqlpp::Connection& connection) {
    auto query = connection.query();
    query << "SHOW TABLES IN " << schema << " LIKE " << mysqlpp::quote << table;
    auto result = query.store();
    return !result.empty();
  }
}

#endif
