#ifndef BEAM_QUERIES_SQL_UTILITIES_HPP
#define BEAM_QUERIES_SQL_UTILITIES_HPP
#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <Viper/Expressions/Expression.hpp>
#include <Viper/Expressions/SqlFunctions.hpp>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Queries/SqlTranslator.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam::Queries {

  //! Builds an SQL expression to test a Range.
  /*!
    \param range The Range to query.
    \return The SQL expression testing within the <i>range</i>.
  */
  inline auto BuildRangeExpression(const Range& range) {
    auto start = boost::get<Sequence>(range.GetStart()).GetOrdinal();
    auto end = boost::get<Sequence>(range.GetEnd()).GetOrdinal();
    return Viper::sym("query_sequence") >= start &&
      Viper::sym("query_sequence") <= end;
  }

  //! Sanitizes a query for use with an SQL database.
  /*!
    \param query The query to sanitize.
    \param table The table to query.
    \param index The SQL expression identifying the query's index.
    \param connectionPool The SQL connection pool used to query.
    \return The sanitized <i>query</i>.
  */
  template<typename Query, typename ConnectionPool>
  Query SanitizeSqlQuery(const Query& query, const std::string& table,
      const Viper::Expression& index, ConnectionPool& connectionPool) {
    auto start = [&] {
      if(auto start = boost::get<Sequence>(&query.GetRange().GetStart())) {
        return *start;
      }
      auto timestamp = boost::get<boost::posix_time::ptime>(
        query.GetRange().GetStart());
      std::vector<std::uint64_t> rows;
      auto connection = connectionPool.Acquire();
      connection->execute(Viper::select(
        Viper::min<std::uint64_t>("query_sequence"), table,
        index && Viper::sym("timestamp") >=
        MySql::ToMySqlTimestamp(timestamp), std::back_inserter(rows)));
      if(rows.empty()) {
        return Sequence::Last();
      }
      return Sequence(rows.front());
    }();
    auto end = [&] {
      if(auto end = boost::get<Sequence>(&query.GetRange().GetEnd())) {
        return *end;
      }
      auto timestamp = boost::get<boost::posix_time::ptime>(
        query.GetRange().GetEnd());
      std::vector<std::uint64_t> rows;
      auto connection = connectionPool.Acquire();
      connection->execute(Viper::select(
        Viper::max<std::uint64_t>("query_sequence"), table,
        index && Viper::sym("timestamp") <=
        MySql::ToMySqlTimestamp(timestamp), std::back_inserter(rows)));
      if(rows.empty()) {
        return Sequence::First();
      }
      return Sequence(rows.front());
    }();
    Query sanitizedQuery = query;
    sanitizedQuery.SetRange(start, end);
    return sanitizedQuery;
  }

#if 0
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
    auto connection = connectionPool.Acquire();
    threadPool.Queue(
      [&] {
        auto sqlQuery = connection->query();
        if(query.empty()) {
          sqlQuery << "SELECT * FROM " << table << " WHERE FALSE";
        } else {
          sqlQuery << "SELECT * FROM " << table << " WHERE " << query;
        }
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
#endif

  //! Loads SequencedValue's from an SQL database.
  /*!
    \param query The query to submit.
    \param row The type of row's to select.
    \param table The name of the table to select from.
    \param index The expression used to identify the index.
    \param threadPool The ThreadPool used to partition the reads.
    \param connectionPool Contains the pool of SQL connections to use.
    \return The list of SequencedValue's satisfying the <i>query</i>.
  */
  template<typename Translator, typename Query, typename Row,
    typename ConnectionPool>
  auto LoadSqlQuery(const Query& query, const Row& row,
      const std::string& table, const Viper::Expression& index,
      Threading::ThreadPool& threadPool, ConnectionPool& connectionPool) {
    using Type = typename Row::Type;
    constexpr auto MAX_READS_PER_QUERY = 1000;
    std::vector<Type> records;
    if(query.GetRange().GetStart() == Sequence::Present() ||
        query.GetRange().GetStart() == Sequence::Last()) {
      return records;
    }
    auto sanitizedQuery = SanitizeSqlQuery(query, table, index, connectionPool);
    auto startPoint =
      boost::get<Sequence>(sanitizedQuery.GetRange().GetStart());
    auto endPoint =
      boost::get<Sequence>(sanitizedQuery.GetRange().GetEnd());
    auto remainingLimit = sanitizedQuery.GetSnapshotLimit().GetSize();
    if(sanitizedQuery.GetSnapshotLimit().GetType() ==
        SnapshotLimit::Type::TAIL) {
      std::vector<std::vector<Type>> partitions;
      while(remainingLimit > 0 && endPoint >= startPoint) {
        Routines::Async<std::vector<Type>> result;
        auto subsetQuery = sanitizedQuery;
        subsetQuery.SetRange(startPoint, endPoint);
        subsetQuery.SetSnapshotLimit(SnapshotLimit::Type::TAIL, remainingLimit);
        auto connection = connectionPool.Acquire();
        threadPool.Queue(
          [&] {
            auto filterQuery = BuildSqlQuery<Translator>(table,
              subsetQuery.GetFilter());
            auto range = BuildRangeExpression(subsetQuery.GetRange());
            auto limit = std::min(MAX_READS_PER_QUERY,
              subsetQuery.GetSnapshotLimit().GetSize());
            std::vector<Type> rows;
            connection->execute(Viper::select(row,
              Viper::select({"*"}, table, index && range,
              Viper::order_by("query_sequence", Viper::Order::DESC),
              Viper::limit(limit)),
              Viper::order_by("query_sequence", Viper::Order::ASC),
              std::back_inserter(rows)));
            return rows;
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
        Routines::Async<std::vector<Type>> result;
        auto connection = connectionPool.Acquire();
        threadPool.Queue(
          [&] {
            auto filterQuery = BuildSqlQuery<Translator>(table,
              subsetQuery.GetFilter());
            auto range = BuildRangeExpression(subsetQuery.GetRange());
            auto limit = std::min(MAX_READS_PER_QUERY,
              subsetQuery.GetSnapshotLimit().GetSize());
            std::vector<Type> rows;
            connection->execute(
              Viper::select(row, table, index && range,
              Viper::order_by("query_sequence", Viper::Order::ASC),
              Viper::limit(limit), std::back_inserter(rows)));
            return rows;
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

#endif
