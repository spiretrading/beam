#ifndef BEAM_SQL_UTILITIES_HPP
#define BEAM_SQL_UTILITIES_HPP
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
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

  inline boost::posix_time::ptime FromDateTime(
      const mysqlpp::DateTime& source) {
    if(source == mysqlpp::DateTime(3999, 1, 1, 1, 1, 1)) {
      return boost::posix_time::pos_infin;
    } else if(source == mysqlpp::DateTime(std::time_t(0))) {
      return boost::posix_time::not_a_date_time;
    }
    std::tm pt_tm;
    pt_tm.tm_year = source.year() - 1900;
    pt_tm.tm_mon = source.month() - 1;
    pt_tm.tm_mday = source.day();
    pt_tm.tm_hour = source.hour();
    pt_tm.tm_min = source.minute();
    pt_tm.tm_sec = source.second();
    return boost::posix_time::ptime_from_tm(pt_tm);
  }

  inline mysqlpp::DateTime ToDateTime(const boost::posix_time::ptime& source) {
    if(source == boost::posix_time::pos_infin) {
      return mysqlpp::DateTime(3999, 1, 1, 1, 1, 1);
    } else if(source.is_special()) {
      return mysqlpp::DateTime(std::time_t(0));
    } else {
      return mysqlpp::DateTime(
        static_cast<unsigned short>(source.date().year()),
        static_cast<unsigned char>(source.date().month()),
        static_cast<unsigned char>(source.date().day()),
        static_cast<unsigned char>(source.time_of_day().hours()),
        static_cast<unsigned char>(source.time_of_day().minutes()),
        static_cast<unsigned char>(source.time_of_day().seconds()));
    }
  }
}

#endif
