#ifndef BEAM_BASIC_QUERY_HPP
#define BEAM_BASIC_QUERY_HPP
#include <ostream>
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/InterruptableQuery.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /**
   * Composes various standard query types into a query with common and basic
   * functionality.
   */
  template<typename T>
  class BasicQuery : public IndexedQuery<T>, public RangedQuery,
      public SnapshotLimitedQuery, public InterruptableQuery,
      public FilteredQuery {
    protected:
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct DataShuttle;
  };

  /**
   * Returns a BasicQuery that streams the most recent value produced.
   * @param index The index to query.
   */
  template<typename Index>
  BasicQuery<Index> make_current_query(Index index) {
    auto query = BasicQuery<Index>();
    query.set_index(std::move(index));
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 1);
    query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
    return query;
  }

  /**
   * Returns a BasicQuery that retrives only the latest value and then breaks.
   * @param index The index to query.
   */
  template<typename Index>
  BasicQuery<Index> make_latest_query(Index index) {
    auto query = BasicQuery<Index>();
    query.set_index(std::move(index));
    query.set_range(Range::HISTORICAL);
    query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 1);
    return query;
  }

  /**
   * Returns a BasicQuery that streams real time values.
   * @param index The index to query.
   */
  template<typename Index>
  BasicQuery<Index> make_real_time_query(Index index) {
    auto query = BasicQuery<Index>();
    query.set_index(std::move(index));
    query.set_range(Range::REAL_TIME);
    query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
    return query;
  }

  template<typename T>
  std::ostream& operator <<(std::ostream& out, const BasicQuery<T>& query) {
    return out << '(' << query.get_index() << ' ' << query.get_range() << ' ' <<
      query.get_snapshot_limit() << ' ' << query.get_interruption_policy() <<
      ' ' << query.get_filter() << ')';
  }

  template<typename T>
  template<IsShuttle S>
  void BasicQuery<T>::shuttle(S& shuttle, unsigned int version) {
    Beam::Shuttle<IndexedQuery<T>>()(shuttle, *this, version);
    Beam::Shuttle<RangedQuery>()(shuttle, *this, version);
    Beam::Shuttle<SnapshotLimitedQuery>()(shuttle, *this, version);
    Beam::Shuttle<InterruptableQuery>()(shuttle, *this, version);
    Beam::Shuttle<FilteredQuery>()(shuttle, *this, version);
  }
}

#endif
