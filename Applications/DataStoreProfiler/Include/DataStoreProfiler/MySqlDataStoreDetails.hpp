#ifndef BEAM_DATASTOREPROFILER_MYSQLDATASTOREDETAILS_HPP
#define BEAM_DATASTOREPROFILER_MYSQLDATASTOREDETAILS_HPP
#include <string>
#include <Beam/MySql/PosixTimeToMySqlDateTime.hpp>
#include <Beam/MySql/Utilities.hpp>
#include <Beam/Queries/SqlUtilities.hpp>
#include <boost/lexical_cast.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "DataStoreProfiler/EntryQuery.hpp"

namespace Beam {
namespace Details {
  sql_create_7(entries, 7, 0,
    mysqlpp::sql_varchar, name,
    mysqlpp::sql_int, item_a,
    mysqlpp::sql_bigint, item_b,
    mysqlpp::sql_bigint, item_c,
    mysqlpp::sql_varchar, item_d,
    mysqlpp::sql_bigint_unsigned, timestamp,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  inline bool LoadEntriesTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "entries", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE entries ("
      "name VARCHAR(16) BINARY NOT NULL,"
      "item_a INTEGER NOT NULL,"
      "item_b BIGINT NOT NULL,"
      "item_c BIGINT NOT NULL,"
      "item_d VARCHAR(256) BINARY NOT NULL,"
      "timestamp BIGINT UNSIGNED NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX sequence_index(name, query_sequence),"
      "INDEX timestamp_index(name, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadEntriesTable(databaseConnection, schema)) {
      return false;
    }
    return true;
  }

  struct SqlFunctor {
    std::string operator ()(const std::string& name) const {
      std::string index = "name = \"";
      index += name;
      index += "\"";
      return index;
    };

    std::string InsertKey(const std::string& name) const {
      return "\"" + name + "\"";
    }

    SequencedEntry operator ()(const entries& row) const {
      auto entry = Queries::MakeSequencedValue(
        Entry{std::string{row.name}, row.item_a, row.item_b, row.item_c,
        row.item_d, MySql::FromMySqlTimestamp(row.timestamp)},
        Queries::Sequence{row.query_sequence});
      return entry;
    }

    entries operator ()(const SequencedIndexedEntry& entry) const {
      entries row{entry->GetIndex(), (*entry)->m_itemA, (*entry)->m_itemB,
        (*entry)->m_itemC, (*entry)->m_itemD,
        MySql::ToMySqlTimestamp((*entry)->m_timestamp),
        entry.GetSequence().GetOrdinal()};
      return row;
    }
  };
}
}

#endif
