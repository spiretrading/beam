#ifndef BEAM_QUERYSQLUTILITIES_HPP
#define BEAM_QUERYSQLUTILITIES_HPP
#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <mysql++/connection.h>
#include <mysql++/query.h>
#include "Beam/MySql/DatabaseConnectionPool.hpp"
#include "Beam/MySql/Utilities.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Queries/SqlTranslator.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam {
namespace Queries {

  //! Loads the initial Sequence to use from an SQL database.
  /*!
    \param table The table to query.
    \param indexQuery The SQL query fragment containing the index to query.
    \param connection The MySQL connection to use.
    \return The initial Sequence number.
  */
  inline Sequence LoadSqlInitialSequence(const std::string& table,
      const std::string& indexQuery, mysqlpp::Connection& connection) {
    auto query = connection.query();
    query << "SELECT query_sequence FROM " << table << " WHERE " <<
      indexQuery << " ORDER BY query_sequence DESC LIMIT 1";
    auto result = query.store();
    if(!result || result.size() != 1) {
      return Sequence(1);
    } else {
      return Increment(Sequence(result[0][0].conv<std::uint64_t>(0)));
    }
  }

  //! Builds an SQL query fragment over a Range.
  /*!
    \param range The Range to query.
    \return The SQL query fragment to query over the specified <i>range</i>.
  */
  inline std::string BuildRangeSqlQueryFragment(const Range& range) {
    auto startSequence = boost::lexical_cast<std::string>(
      boost::get<Sequence>(range.GetStart()).GetOrdinal());
    auto endSequence = boost::lexical_cast<std::string>(
      boost::get<Sequence>(range.GetEnd()).GetOrdinal());
    auto query = "(query_sequence >= " + startSequence +
      " AND query_sequence <= " + endSequence + ")";
    return query;
  }

  //! Loads the table storing hints used to retrieve query Sequences, creating
  //! it if it doesn't yet exist.
  /*!
    \param schema The schema to load the table from.
    \param table The name of the table to load.
    \param indexFragment Specifies the index to use.
    \param connection The SQL connection.
    \return <code>true</code> iff the table was loaded.
  */
  inline bool LoadSequencesTable(const std::string& schema,
      const std::string& table, const std::string& indexFragment,
      mysqlpp::Connection& connection) {
    if(MySql::TestTable(schema, table + "_sequences", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE " << (table + "_sequences") << ("("
      "timestamp BIGINT UNSIGNED NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL," +
      indexFragment +
      ", INDEX(timestamp))");
    return query.execute();
  }

  //! Queries an SQL database for a Range of Sequences matching a timestamp.
  /*!
    \param timestamp The timestamp to search for.
    \param table The table query.
    \param indexQuery The SQL query fragment containing the index.
    \param connection The SQL connection.
    \return The Range of Sequences matching the <i>timestamp</i>.
  */
  inline Range LoadSequenceHint(const boost::posix_time::ptime& timestamp,
      const std::string& table, const std::string& indexQuery,
      mysqlpp::Connection& connection) {
    auto sqlQuery = connection.query();
    sqlQuery << "SELECT MAX(query_sequence) FROM " << table << " WHERE " <<
      indexQuery << " AND timestamp <= " << MySql::ToMySqlTimestamp(timestamp);
    auto result = sqlQuery.store();
    Sequence start;
    if(!result || result.size() != 1 || result[0][0].is_null()) {
      start = Sequence::First();
    } else {
      start = Sequence(result[0][0].conv<std::uint64_t>(0));
    }
    sqlQuery.reset();
    sqlQuery << "SELECT MIN(query_sequence) FROM " << table << " WHERE " <<
      indexQuery << " AND timestamp >= " << MySql::ToMySqlTimestamp(timestamp);
    result = sqlQuery.store();
    Sequence end;
    if(!result || result.size() != 1 || result[0][0].is_null()) {
      end = Sequence::Last();
    } else {
      end = Sequence(result[0][0].conv<std::uint64_t>(0));
    }
    return Range(start, end);
  }

  //! Stores a Sequence hint for a value.
  /*!
    \param table The table to store the hint in.
    \param value The value providing the Sequence and timestamp hint.
    \param index The hint's SQL index.
    \param connection The MySQL connection to use.
  */
  template<typename T>
  void StoreSequenceHint(const std::string& table, const T& value,
      const std::string& index, mysqlpp::Connection& connection) {
    auto sqlQuery = connection.query();
    sqlQuery << "INSERT INTO " << table << " VALUES (" <<
      MySql::ToMySqlTimestamp(GetTimestamp(**value)) << ", " <<
      value.GetSequence().GetOrdinal() << ", " << index << ")";
    sqlQuery.execute();
  };

  //! Sanitizes a query for use with an SQL database.
  /*!
    \param query The query to sanitize.
    \param table The table to query.
    \param indexQuery The SQL query fragment containing the index.
    \param connectionPool The SQL connection pool used to query.
    \return The sanitized <i>query</i>.
  */
  template<typename Query>
  Query SanitizeSqlQuery(const Query& query, const std::string& table,
      const std::string& indexQuery,
      MySql::DatabaseConnectionPool& connectionPool) {
    Range::Point start;
    if(auto queryStartPoint = boost::get<Sequence>(
        &query.GetRange().GetStart())) {
      start = *queryStartPoint;
    } else {
      auto connection = connectionPool.Acquire();
      auto timestamp = boost::get<boost::posix_time::ptime>(
        query.GetRange().GetStart());
      auto rangeHint = LoadSequenceHint(timestamp, table + "_sequences",
        indexQuery, *connection);
      auto sqlQuery = connection->query();
      sqlQuery << "SELECT MIN(query_sequence) FROM " << table << " WHERE " <<
        indexQuery << " AND query_sequence >= " <<
        boost::get<Sequence>(rangeHint.GetStart()).GetOrdinal() <<
        " AND query_sequence <= " <<
        boost::get<Sequence>(rangeHint.GetEnd()).GetOrdinal() <<
        " AND timestamp >= " << MySql::ToMySqlTimestamp(timestamp);
      auto result = sqlQuery.store();
      if(!result || result.size() != 1 || result[0][0].is_null()) {
        start = Sequence::Last();
      } else {
        start = Sequence(result[0][0].conv<std::uint64_t>(0));
      }
    }
    Range::Point end;
    if(auto queryEndPoint = boost::get<Sequence>(&query.GetRange().GetEnd())) {
      end = *queryEndPoint;
    } else {
      auto connection = connectionPool.Acquire();
      auto timestamp = boost::get<boost::posix_time::ptime>(
        query.GetRange().GetEnd());
      auto rangeHint = LoadSequenceHint(timestamp, table + "_sequences",
        indexQuery, *connection);
      auto sqlQuery = connection->query();
      sqlQuery << "SELECT MAX(query_sequence) FROM " << table << " WHERE " <<
        indexQuery << " AND timestamp <= " <<
        MySql::ToMySqlTimestamp(timestamp) << " AND query_sequence >= " <<
        boost::get<Sequence>(rangeHint.GetStart()).GetOrdinal() <<
        " AND query_sequence <= " <<
        boost::get<Sequence>(rangeHint.GetEnd()).GetOrdinal();
      auto result = sqlQuery.store();
      if(!result || result.size() != 1 || result[0][0].is_null()) {
        end = Sequence::First();
      } else {
        end = Sequence(result[0][0].conv<std::uint64_t>(0));
      }
    }
    Query sanitizedQuery = query;
    sanitizedQuery.SetRange(start, end);
    return sanitizedQuery;
  }

  //! Loads SequencedValue's from an SQL database.
  /*!
    \param query The query to submit.
    \param table The SQL table to load the data from.
    \param threadPool The ThreadPool used to partition the reads.
    \param connectionPool Contains the pool of SQL connections to use.
    \param rowToData Used to convert an SQL row to the SequencedValue.
    \return The list of SequencedValue's satisfying the <i>query</i>.
  */
  template<typename T, typename Row, typename F>
  std::vector<T> LoadSqlQuery(const std::string& query,
      const std::string& table, Threading::ThreadPool& threadPool,
      MySql::DatabaseConnectionPool& connectionPool, F rowToData) {
    std::vector<T> records;
    Routines::Async<std::vector<T>> result;
    threadPool.Queue(
      [&] {
        auto connection = connectionPool.Acquire();
        auto sqlQuery = connection->query();
        sqlQuery << "SELECT * FROM " << table << " WHERE " << query;
        std::vector<Row> rows;
        sqlQuery.storein(rows);
        if(sqlQuery.errnum() != 0) {
          std::stringstream ss;
          ss << sqlQuery.error() << " : " << sqlQuery.str();
          BOOST_THROW_EXCEPTION(std::runtime_error(ss.str()));
        }
        sqlQuery.reset();
        std::vector<T> transformedRows;
        std::transform(rows.begin(), rows.end(),
          std::back_inserter(transformedRows), rowToData);
        return transformedRows;
      }, result.GetEval());
    return result.Get();
  }

  //! Loads the minimum Sequence stored for a given index.
  /*!
    \param table The SQL table to query.
    \param indexQuery The SQL query fragment containing the data's index.
    \param connection The SQL connect to use for the query.
    \return The minimum Sequence stored.
  */
  inline Sequence GetMinSequence(const std::string& table,
      const std::string& indexQuery, mysqlpp::Connection& connection) {
    auto sqlQuery = connection.query();
    sqlQuery << "SELECT MIN(query_sequence) FROM " << table << " WHERE " <<
      indexQuery;
    auto result = sqlQuery.store();
    if(!result || result.size() != 1 || result[0][0].is_null()) {
      return Sequence(1);
    } else {
      return Sequence(result[0][0].conv<std::uint64_t>(0));
    }
  }

  //! Loads the maximum Sequence stored for a given index.
  /*!
    \param table The SQL table to query.
    \param indexQuery The SQL query fragment containing the data's index.
    \param connection The SQL connect to use for the query.
    \return The maximum Sequence stored.
  */
  inline Sequence GetMaxSequence(const std::string& table,
      const std::string& indexQuery, mysqlpp::Connection& connection) {
    auto sqlQuery = connection.query();
    sqlQuery << "SELECT MAX(query_sequence) FROM " << table << " WHERE " <<
      indexQuery;
    auto result = sqlQuery.store();
    if(!result || result.size() != 1 || result[0][0].is_null()) {
      return Sequence(1);
    } else {
      return Sequence(result[0][0].conv<std::uint64_t>(0));
    }
  }

  //! Loads SequencedValue's from an SQL database.
  /*!
    \param query The query to submit.
    \param table The SQL table to load the data from.
    \param indexQuery The SQL query fragment containing the data's index.
    \param threadPool The ThreadPool used to partition the reads.
    \param connectionPool Contains the pool of SQL connections to use.
    \param rowToData Used to convert an SQL row to the SequencedValue.
    \return The list of SequencedValue's satisfying the <i>query</i>.
  */
  template<typename T, typename Row, typename QueryTranslator, typename Query,
    typename F>
  std::vector<T> LoadSqlQuery(const Query& query, const std::string& table,
      const std::string& indexQuery, Threading::ThreadPool& threadPool,
      MySql::DatabaseConnectionPool& connectionPool, F rowToData) {
    const int MAX_READS_PER_QUERY = 1000;
    std::vector<T> records;
    if(query.GetRange().GetStart() == Sequence::Present() ||
        query.GetRange().GetStart() == Sequence::Last()) {
      return records;
    }
    auto sanitizedQuery = SanitizeSqlQuery(query, table, indexQuery,
      connectionPool);
    auto startPoint =
      boost::get<Sequence>(sanitizedQuery.GetRange().GetStart());
    auto endPoint =
      boost::get<Sequence>(sanitizedQuery.GetRange().GetEnd());
    auto remainingLimit = sanitizedQuery.GetSnapshotLimit().GetSize();
    if(sanitizedQuery.GetSnapshotLimit().GetType() ==
        SnapshotLimit::Type::TAIL) {
      std::vector<std::vector<T>> partitions;
      while(remainingLimit > 0 && endPoint >= startPoint) {
        Routines::Async<std::vector<T>> result;
        auto subsetQuery = sanitizedQuery;
        subsetQuery.SetRange(startPoint, endPoint);
        subsetQuery.SetSnapshotLimit(SnapshotLimit::Type::TAIL, remainingLimit);
        threadPool.Queue(
          [&] {
            auto filterQuery = BuildSqlQuery<QueryTranslator>(
              table, subsetQuery.GetFilter());
            if(filterQuery.empty()) {
              filterQuery = "1";
            }
            auto rangeQuery = BuildRangeSqlQueryFragment(
              subsetQuery.GetRange());
            auto limit = std::min(MAX_READS_PER_QUERY,
              subsetQuery.GetSnapshotLimit().GetSize());
            auto connection = connectionPool.Acquire();
            auto sqlQuery = connection->query();
            sqlQuery << "SELECT result.* FROM (SELECT * FROM " << table <<
              " WHERE (" << indexQuery << ") AND (" << rangeQuery <<
              ") AND (" << filterQuery <<
              ") ORDER BY query_sequence DESC LIMIT " << limit <<
              ") AS result ORDER BY query_sequence ASC";
            std::vector<Row> rows;
            sqlQuery.storein(rows);
            if(sqlQuery.errnum() != 0) {
              std::stringstream ss;
              ss << sqlQuery.error() << " : " << sqlQuery.str();
              BOOST_THROW_EXCEPTION(std::runtime_error(ss.str()));
            }
            sqlQuery.reset();
            std::vector<T> transformedRows;
            std::transform(rows.begin(), rows.end(),
              std::back_inserter(transformedRows), rowToData);
            return transformedRows;
          }, result.GetEval());
        partitions.push_back(std::move(result.Get()));
        remainingLimit -= partitions.back().size();
        if(partitions.back().empty() ||
            partitions.back().front().GetSequence() == Sequence::First()) {
          remainingLimit = 0;
        } else {
          endPoint = Decrement(partitions.back().front().GetSequence());
        }
      }
      for(const auto& partition : boost::adaptors::reverse(partitions)) {
        records.insert(records.end(), partition.begin(), partition.end());
      }
    } else {
      while(remainingLimit > 0 && startPoint <= endPoint) {
        auto subsetQuery = sanitizedQuery;
        subsetQuery.SetRange(startPoint, endPoint);
        if(sanitizedQuery.GetSnapshotLimit() != SnapshotLimit::Unlimited()) {
          subsetQuery.SetSnapshotLimit(SnapshotLimit::Type::HEAD,
            remainingLimit);
        }
        Routines::Async<std::vector<T>> result;
        threadPool.Queue(
          [&] {
            auto filterQuery = BuildSqlQuery<QueryTranslator>(
              table, subsetQuery.GetFilter());
            if(filterQuery.empty()) {
              filterQuery = "1";
            }
            auto rangeQuery = BuildRangeSqlQueryFragment(
              subsetQuery.GetRange());
            auto limit = std::min(MAX_READS_PER_QUERY,
              subsetQuery.GetSnapshotLimit().GetSize());
            auto connection = connectionPool.Acquire();
            auto sqlQuery = connection->query();
            sqlQuery << "SELECT * FROM " << table << " WHERE (" <<
              indexQuery << ") AND (" << rangeQuery << ") AND (" <<
              filterQuery << ") ORDER BY query_sequence ASC LIMIT " << limit;
            std::vector<Row> rows;
            sqlQuery.storein(rows);
            if(sqlQuery.errnum() != 0) {
              std::stringstream ss;
              ss << sqlQuery.error() << " : " << sqlQuery.str();
              BOOST_THROW_EXCEPTION(std::runtime_error(ss.str()));
            }
            sqlQuery.reset();
            std::vector<T> transformedRows;
            std::transform(rows.begin(), rows.end(),
              std::back_inserter(transformedRows), rowToData);
            return transformedRows;
          }, result.GetEval());
        auto partition = std::move(result.Get());
        if(sanitizedQuery.GetSnapshotLimit() != SnapshotLimit::Unlimited()) {
          remainingLimit -= partition.size();
        }
        records.insert(records.end(), partition.begin(), partition.end());
        if(partition.empty() ||
            partition.back().GetSequence() == Sequence::Last()) {
          remainingLimit = 0;
        } else {
          startPoint = Increment(partition.back().GetSequence());
        }
      }
    }
    return records;
  }
}
}

#endif
