#ifndef BEAM_QUERIES_SQL_UTILITIES_HPP
#define BEAM_QUERIES_SQL_UTILITIES_HPP
#include <string>
#include <utility>
#include <vector>
#include <boost/range/adaptor/reversed.hpp>
#include <Viper/Expressions/Expression.hpp>
#include <Viper/Expressions/SqlFunctions.hpp>
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Queries/SqlTranslator.hpp"
#include "Beam/Sql/DatabaseConnectionPool.hpp"

namespace Beam {

  /**
   * Returns an SQL expression to test a Range.
   * @param range The Range to query.
   * @return The SQL expression testing within the <i>range</i>.
   */
  inline auto make_range_expression(const Range& range) {
    auto start = boost::get<Sequence>(range.get_start()).get_ordinal();
    auto end = boost::get<Sequence>(range.get_end()).get_ordinal();
    return Viper::sym("query_sequence") >= start &&
      Viper::sym("query_sequence") <= end;
  }

  /**
   * Sanitizes a query for use with an SQL database.
   * @param query The query to sanitize.
   * @param table The table to query.
   * @param index The SQL expression identifying the query's index.
   * @param connection_pool The SQL connection pool used to query.
   * @return The sanitized <i>query</i>.
   */
  template<typename Query, typename C>
  auto sanitize(Query query, const std::string& table,
      const Viper::Expression& index,
      DatabaseConnectionPool<C>& connection_pool) {
    auto start = [&] {
      if(auto start = boost::get<Sequence>(&query.get_range().get_start())) {
        return *start;
      }
      auto timestamp =
        boost::get<boost::posix_time::ptime>(query.get_range().get_start());
      auto sequence = std::optional<std::uint64_t>();
      auto connection = connection_pool.load();
      connection->execute(Viper::select(
        Viper::min<std::uint64_t>("query_sequence"), table,
        index && Viper::sym("timestamp") >= timestamp, &sequence));
      if(sequence) {
        return Sequence(*sequence);
      }
      return Sequence::LAST;
    }();
    auto end = [&] {
      if(auto end = boost::get<Sequence>(&query.get_range().get_end())) {
        return *end;
      }
      auto timestamp =
        boost::get<boost::posix_time::ptime>(query.get_range().get_end());
      auto sequence = std::optional<std::uint64_t>();
      auto connection = connection_pool.load();
      connection->execute(Viper::select(
        Viper::max<std::uint64_t>("query_sequence"), table,
        index && Viper::sym("timestamp") <= timestamp, &sequence));
      if(sequence) {
        return Sequence(*sequence);
      }
      return Sequence::FIRST;
    }();
    query.set_range(start, end);
    return query;
  }

  /**
   * Loads SequencedValue's from an SQL database.
   * @param expression The SQL expression to query.
   * @param row The type of row's to select.
   * @param table The SQL table to load the data from.
   * @param connection_pool Contains the pool of SQL connections to use.
   * @return The list of SequencedValue's satisfying the <i>expression</i>.
   */
  template<typename Row, typename C>
  auto load_sql_query(const Viper::Expression& expression, const Row& row,
      const std::string& table,
      DatabaseConnectionPool<C>& connection_pool) {
    using Type = typename Row::Type;
    auto connection = connection_pool.load();
    auto rows = std::vector<Type>();
    connection->execute(Viper::select(row, table, expression,
      Viper::order_by("query_sequence", Viper::Order::ASC),
      std::back_inserter(rows)));
    return rows;
  }

  /**
   * Loads SequencedValue's from an SQL database.
   * @param query The query to submit.
   * @param row The type of row's to select.
   * @param table The name of the table to select from.
   * @param index The expression used to identify the index.
   * @param connection_pool Contains the pool of SQL connections to use.
   * @return The list of SequencedValue's satisfying the <i>query</i>.
   */
  template<typename Translator, typename Query, typename Row, typename C>
  auto load_sql_query(Query query, const Row& row, const std::string& table,
      const Viper::Expression& index,
      DatabaseConnectionPool<C>& connection_pool) {
    using Type = typename Row::Type;
    const auto MAX_READS_PER_QUERY = 1000;
    auto records = std::vector<Type>();
    if(query.get_range().get_start() == Sequence::PRESENT ||
        query.get_range().get_start() == Sequence::LAST) {
      return records;
    }
    query = sanitize(std::move(query), table, index, connection_pool);
    auto start = boost::get<Sequence>(query.get_range().get_start());
    auto end = boost::get<Sequence>(query.get_range().get_end());
    auto remaining_limit = query.get_snapshot_limit().get_size();
    if(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::TAIL) {
      auto partitions = std::vector<std::vector<Type>>();
      while(remaining_limit > 0 && end >= start) {
        auto subset_query = query;
        subset_query.set_range(start, end);
        subset_query.set_snapshot_limit(
          SnapshotLimit::Type::TAIL, remaining_limit);
        auto filter =
          make_sql_query<Translator>(table, subset_query.get_filter());
        auto range = make_range_expression(subset_query.get_range());
        auto limit = std::min(
          MAX_READS_PER_QUERY, subset_query.get_snapshot_limit().get_size());
        auto connection = connection_pool.load();
        auto partition = std::vector<Type>();
        connection->execute(Viper::select(row,
          Viper::select({"*"}, table, index && range && filter,
            Viper::order_by("query_sequence", Viper::Order::DESC),
            Viper::limit(limit)),
          Viper::order_by("query_sequence", Viper::Order::ASC),
          std::back_inserter(partition)));
        partitions.push_back(std::move(partition));
        remaining_limit -= partitions.back().size();
        if(partitions.back().empty() ||
            partitions.back().front().get_sequence() == Sequence::FIRST) {
          remaining_limit = 0;
        } else {
          end = decrement(partitions.back().front().get_sequence());
        }
      }
      for(auto& partition : boost::adaptors::reverse(partitions)) {
        records.insert(records.end(), partition.begin(), partition.end());
      }
    } else {
      while(remaining_limit > 0 && start <= end) {
        auto subset_query = query;
        subset_query.set_range(start, end);
        if(query.get_snapshot_limit() != SnapshotLimit::UNLIMITED) {
          subset_query.set_snapshot_limit(
            SnapshotLimit::Type::HEAD, remaining_limit);
        }
        auto filter =
          make_sql_query<Translator>(table, subset_query.get_filter());
        auto range = make_range_expression(subset_query.get_range());
        auto limit = std::min(
          MAX_READS_PER_QUERY, subset_query.get_snapshot_limit().get_size());
        auto connection = connection_pool.load();
        auto partition = std::vector<Type>();
        connection->execute(Viper::select(row, table, index && range && filter,
          Viper::order_by("query_sequence", Viper::Order::ASC),
          Viper::limit(limit), std::back_inserter(partition)));
        if(query.get_snapshot_limit() != SnapshotLimit::UNLIMITED) {
          remaining_limit -= partition.size();
        }
        records.insert(records.end(), partition.begin(), partition.end());
        if(partition.empty() ||
            partition.back().get_sequence() == Sequence::LAST) {
          remaining_limit = 0;
        } else {
          start = increment(partition.back().get_sequence());
        }
      }
    }
    return records;
  }
}

#endif
