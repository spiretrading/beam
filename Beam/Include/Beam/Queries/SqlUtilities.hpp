#ifndef BEAM_QUERIES_SQL_UTILITIES_HPP
#define BEAM_QUERIES_SQL_UTILITIES_HPP
#include <string>
#include <utility>
#include <vector>
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
  auto SanitizeSqlQuery(Query query, const std::string& table,
      const Viper::Expression& index, ConnectionPool& connectionPool) {
    auto start = [&] {
      if(auto start = boost::get<Sequence>(&query.GetRange().GetStart())) {
        return *start;
      }
      auto timestamp = boost::get<boost::posix_time::ptime>(
        query.GetRange().GetStart());
      auto sequence = std::optional<std::uint64_t>();
      auto connection = connectionPool.Acquire();
      connection->execute(Viper::select(
        Viper::min<std::uint64_t>("query_sequence"), table,
        index && Viper::sym("timestamp") >= timestamp, &sequence));
      if(sequence.has_value()) {
        return Sequence(*sequence);
      }
      return Sequence::Last();
    }();
    auto end = [&] {
      if(auto end = boost::get<Sequence>(&query.GetRange().GetEnd())) {
        return *end;
      }
      auto timestamp = boost::get<boost::posix_time::ptime>(
        query.GetRange().GetEnd());
      auto sequence = std::optional<std::uint64_t>();
      auto connection = connectionPool.Acquire();
      connection->execute(Viper::select(
        Viper::max<std::uint64_t>("query_sequence"), table,
        index && Viper::sym("timestamp") <= timestamp, &sequence));
      if(sequence.has_value()) {
        return Sequence(*sequence);
      }
      return Sequence::First();
    }();
    auto sanitizedQuery = std::move(query);
    sanitizedQuery.SetRange(start, end);
    return sanitizedQuery;
  }

  //! Loads SequencedValue's from an SQL database.
  /*!
    \param expression The SQL expression to query.
    \param row The type of row's to select.
    \param table The SQL table to load the data from.
    \param threadPool The ThreadPool used to partition the reads.
    \param connectionPool Contains the pool of SQL connections to use.
    \return The list of SequencedValue's satisfying the <i>expression</i>.
  */
  template<typename Row, typename ConnectionPool>
  auto LoadSqlQuery(const Viper::Expression& expression, const Row& row,
      const std::string& table, Threading::ThreadPool& threadPool,
      ConnectionPool& connectionPool) {
    using Type = typename Row::Type;
    auto result = Routines::Async<std::vector<Type>>();
    auto connection = connectionPool.Acquire();
    threadPool.Queue(
      [&] {
        auto rows = std::vector<Type>();
        connection->execute(Viper::select(row, table, expression,
          Viper::order_by("query_sequence", Viper::Order::ASC),
          std::back_inserter(rows)));
        return rows;
      }, result.GetEval());
    return result.Get();
  }

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
  auto LoadSqlQuery(Query query, const Row& row, const std::string& table,
      const Viper::Expression& index, Threading::ThreadPool& threadPool,
      ConnectionPool& connectionPool) {
    using Type = typename Row::Type;
    constexpr auto MAX_READS_PER_QUERY = 1000;
    auto records = std::vector<Type>();
    if(query.GetRange().GetStart() == Sequence::Present() ||
        query.GetRange().GetStart() == Sequence::Last()) {
      return records;
    }
    auto sanitizedQuery = SanitizeSqlQuery(std::move(query), table, index,
      connectionPool);
    auto startPoint =
      boost::get<Sequence>(sanitizedQuery.GetRange().GetStart());
    auto endPoint = boost::get<Sequence>(sanitizedQuery.GetRange().GetEnd());
    auto remainingLimit = sanitizedQuery.GetSnapshotLimit().GetSize();
    if(sanitizedQuery.GetSnapshotLimit().GetType() ==
        SnapshotLimit::Type::TAIL) {
      auto partitions = std::vector<std::vector<Type>>();
      while(remainingLimit > 0 && endPoint >= startPoint) {
        auto result = Routines::Async<std::vector<Type>>();
        auto subsetQuery = sanitizedQuery;
        subsetQuery.SetRange(startPoint, endPoint);
        subsetQuery.SetSnapshotLimit(SnapshotLimit::Type::TAIL, remainingLimit);
        auto connection = connectionPool.Acquire();
        threadPool.Queue(
          [&] {
            auto filter = BuildSqlQuery<Translator>(table,
              subsetQuery.GetFilter());
            auto range = BuildRangeExpression(subsetQuery.GetRange());
            auto limit = std::min(MAX_READS_PER_QUERY,
              subsetQuery.GetSnapshotLimit().GetSize());
            auto rows = std::vector<Type>();
            connection->execute(Viper::select(row,
              Viper::select({"*"}, table, index && range && filter,
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
      for(auto& partition : boost::adaptors::reverse(partitions)) {
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
        auto result = Routines::Async<std::vector<Type>>();
        auto connection = connectionPool.Acquire();
        threadPool.Queue(
          [&] {
            auto filter = BuildSqlQuery<Translator>(table,
              subsetQuery.GetFilter());
            auto range = BuildRangeExpression(subsetQuery.GetRange());
            auto limit = std::min(MAX_READS_PER_QUERY,
              subsetQuery.GetSnapshotLimit().GetSize());
            auto rows = std::vector<Type>();
            connection->execute(
              Viper::select(row, table, index && range && filter,
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
