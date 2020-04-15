#ifndef BEAM_QUERIES_TESTS_TEST_ENTRY_HPP
#define BEAM_QUERIES_TESTS_TEST_ENTRY_HPP
#include <ostream>
#include <string>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/QueriesTests/QueriesTests.hpp"

namespace Beam::Queries::Tests {

  /** Represents a data store entry used for testing purposes. */
  struct TestEntry {

    /** The test value to store. */
    int m_value;

    /** The test timestamp. */
    boost::posix_time::ptime m_timestamp;

    bool operator ==(const TestEntry& rhs) const;

    bool operator !=(const TestEntry& rhs) const;
  };

  using SequencedTestEntry = SequencedValue<TestEntry>;
  using SequencedIndexedTestEntry =
    SequencedValue<IndexedValue<TestEntry, std::string>>;

  std::ostream& operator <<(std::ostream& out, const TestEntry& entry);

  template<typename DataStore>
  SequencedIndexedTestEntry StoreValue(DataStore& dataStore, std::string index,
      int value, boost::posix_time::ptime timestamp,
      Beam::Queries::Sequence sequence) {
    auto entry = SequencedValue(IndexedValue(TestEntry{value, timestamp},
      std::move(index)), sequence);
    dataStore.Store(entry);
    return entry;
  }

  template<typename DataStore>
  void TestQuery(DataStore& dataStore, std::string index,
      const Beam::Queries::Range& range, const SnapshotLimit& limit,
      const std::vector<SequencedTestEntry>& expectedResult) {
    auto query = BasicQuery<std::string>();
    query.SetIndex(std::move(index));
    query.SetRange(range);
    query.SetSnapshotLimit(limit);
    auto queryResult = dataStore.Load(query);
    REQUIRE(expectedResult == queryResult);
  }
}

#endif
