#ifndef BEAM_DATA_STORE_PROFILER_ENTRY_QUERY_HPP
#define BEAM_DATA_STORE_PROFILER_ENTRY_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "DataStoreProfiler/Entry.hpp"

namespace Beam {
  using SequencedEntry = SequencedValue<Entry>;
  using IndexedEntry = IndexedValue<Entry, std::string>;
  using SequencedIndexedEntry = SequencedValue<IndexedEntry>;
  using EntryQuery = BasicQuery<std::string>;

  /**
   * Builds an EntryQuery for real time data with a snapshot containing the
   * most recent value.
   * @param name The name of the entry to query.
   */
  inline auto query_real_time_with_snapshot(std::string name) {
    auto query = EntryQuery();
    query.set_index(std::move(name));
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 1);
    query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
    return query;
  }
}

#endif
