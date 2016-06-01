#ifndef BEAM_DATASTOREPROFILER_ENTRYQUERY_HPP
#define BEAM_DATASTOREPROFILER_ENTRYQUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "DataStoreProfiler/Entry.hpp"

namespace Beam {
  using SequencedEntry = Queries::SequencedValue<Entry>;
  using IndexedEntry = Queries::IndexedValue<Entry, std::string>;
  using SequencedIndexedEntry = Queries::SequencedValue<IndexedEntry>;
  using EntryQuery = Queries::BasicQuery<std::string>;

  //! Builds an EntryQuery for real time data with a snapshot containing the
  //! most recent value.
  /*!
    \param name The name of the entry to query.
  */
  inline EntryQuery QueryRealTimeWithSnapshot(std::string name) {
    EntryQuery query;
    query.SetIndex(std::move(name));
    query.SetRange(Queries::Range::Total());
    query.SetSnapshotLimit(Queries::SnapshotLimit::Type::TAIL, 1);
    query.SetInterruptionPolicy(Queries::InterruptionPolicy::IGNORE_CONTINUE);
    return query;
  }
}

#endif
