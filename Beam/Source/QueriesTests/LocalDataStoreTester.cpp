#include <doctest/doctest.h>
#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Entry {
    int m_value;
    ptime m_timestamp;

    Entry() = default;
    Entry(int value, ptime timestamp);
    bool operator ==(const Entry& rhs) const;
  };

  using DataStore = LocalDataStore<BasicQuery<std::string>, Entry,
    EvaluatorTranslator<QueryTypes>>;
  using SequencedEntry = SequencedValue<Entry>;
  using SequencedIndexedEntry = SequencedValue<IndexedValue<Entry,
    std::string>>;

  Entry::Entry(int value, ptime timestamp)
    : m_value(value),
      m_timestamp(timestamp) {}

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }

  SequencedIndexedEntry StoreValue(DataStore& dataStore, std::string index,
      int value, const ptime& timestamp,
      const Beam::Queries::Sequence& sequence) {
    auto entry = SequencedValue(IndexedValue(Entry(value, timestamp), index),
      sequence);
    dataStore.Store(entry);
    return entry;
  }

  void TestQuery(DataStore& dataStore, std::string index,
      const Beam::Queries::Range& range, const SnapshotLimit& limit,
      const std::vector<SequencedEntry>& expectedResult) {
    auto query = BasicQuery<std::string>();
    query.SetIndex(index);
    query.SetRange(range);
    query.SetSnapshotLimit(limit);
    auto queryResult = dataStore.Load(query);
    REQUIRE(expectedResult == queryResult);
  }
}

TEST_SUITE("LocalDataStore") {
  TEST_CASE("store_and_load") {
    auto dataStore = DataStore();
    auto timeClient = IncrementalTimeClient();
    auto sequence = Beam::Queries::Sequence(5);
    auto entryA = StoreValue(dataStore, "hello", 100, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    auto entryB = StoreValue(dataStore, "hello", 200, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    auto entryC = StoreValue(dataStore, "hello", 300, timeClient.GetTime(),
      sequence);
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit::Unlimited(), {entryA, entryB, entryC});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 0), {});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 1), {entryA});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 2), {entryA, entryB});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 3), {entryA, entryB, entryC});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 4), {entryA, entryB, entryC});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 0), {});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryC});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 2), {entryB, entryC});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 3), {entryA, entryB, entryC});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 4), {entryA, entryB, entryC});
  }

  TEST_CASE("load_all") {
    auto dataStore = DataStore();
    auto timeClient = IncrementalTimeClient();
    auto valueA = SequencedValue(IndexedValue(Entry(5, timeClient.GetTime()),
      "hello"), Beam::Queries::Sequence(1));
    dataStore.Store(valueA);
    auto valueB = SequencedValue(IndexedValue(Entry(6, timeClient.GetTime()),
      "hello"), Beam::Queries::Sequence(2));
    dataStore.Store(valueB);
    auto valueC = SequencedValue(IndexedValue(Entry(7, timeClient.GetTime()),
      "goodbye"), Beam::Queries::Sequence(1));
    dataStore.Store(valueC);
    auto valueD = SequencedValue(IndexedValue(Entry(8, timeClient.GetTime()),
      "goodbye"), Beam::Queries::Sequence(2));
    dataStore.Store(valueD);
    auto entries = dataStore.LoadAll();
    auto expectedEntries = std::vector{valueA, valueB, valueC, valueD};
    REQUIRE(entries.size() == expectedEntries.size());
    while(!expectedEntries.empty()) {
      auto expectedEntry = expectedEntries.back();
      REQUIRE(std::find(entries.begin(), entries.end(), expectedEntry) !=
        entries.end());
      expectedEntries.pop_back();
    }
  }
}
