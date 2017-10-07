#include "Beam/QueriesTests/LocalDataStoreTester.hpp"
#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

namespace {
  struct Entry {
    int m_value;
    ptime m_timestamp;

    Entry();
    Entry(int value, ptime timestamp);
    bool operator ==(const Entry& rhs) const;
  };

  typedef LocalDataStore<BasicQuery<string>, Entry,
    EvaluatorTranslator<QueryTypes>> TestDataStore;
  typedef SequencedValue<Entry> SequencedEntry;
  typedef SequencedValue<IndexedValue<Entry, string>> SequencedIndexedEntry;

  Entry::Entry() {}

  Entry::Entry(int value, ptime timestamp)
      : m_value(value),
        m_timestamp(timestamp) {}

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }

  SequencedIndexedEntry StoreValue(TestDataStore& dataStore, string index,
      int value, const ptime& timestamp,
      const Beam::Queries::Sequence& sequence) {
    auto entry = MakeSequencedValue(MakeIndexedValue(Entry(value, timestamp),
      index), sequence);
    dataStore.Store(entry);
    return entry;
  }

  void TestQuery(TestDataStore& dataStore, string index,
      const Beam::Queries::Range& range, const SnapshotLimit& limit,
      const std::vector<SequencedEntry>& expectedResult) {
    BasicQuery<string> query;
    query.SetIndex(index);
    query.SetRange(range);
    query.SetSnapshotLimit(limit);
    auto queryResult = dataStore.Load(query);
    CPPUNIT_ASSERT(expectedResult == queryResult);
  }
}

void LocalDataStoreTester::TestStoreAndLoad() {
  TestDataStore dataStore;
  IncrementalTimeClient timeClient;
  Beam::Queries::Sequence sequence(5);
  SequencedIndexedEntry entryA = StoreValue(dataStore, "hello", 100,
    timeClient.GetTime(), sequence);
  sequence = Increment(sequence);
  SequencedIndexedEntry entryB = StoreValue(dataStore, "hello", 200,
    timeClient.GetTime(), sequence);
  sequence = Increment(sequence);
  SequencedIndexedEntry entryC = StoreValue(dataStore, "hello", 300,
    timeClient.GetTime(), sequence);
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit::Unlimited(), {entryA, entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 0),
    std::vector<SequencedEntry>());
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 1), {entryA});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 2), {entryA, entryB});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 3), {entryA, entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 4), {entryA, entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 0),
    std::vector<SequencedEntry>());
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 2), {entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 3), {entryA, entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 4), {entryA, entryB, entryC});
}

void LocalDataStoreTester::TestLoadAll() {
  TestDataStore dataStore;
  IncrementalTimeClient timeClient;
  auto valueA = MakeSequencedValue(MakeIndexedValue(
    Entry(5, timeClient.GetTime()), "hello"), Sequence(1));
  dataStore.Store(valueA);
  auto valueB = MakeSequencedValue(MakeIndexedValue(
    Entry(6, timeClient.GetTime()), "hello"), Sequence(2));
  dataStore.Store(valueB);
  auto valueC = MakeSequencedValue(MakeIndexedValue(
    Entry(7, timeClient.GetTime()), "goodbye"), Sequence(1));
  dataStore.Store(valueC);
  auto valueD = MakeSequencedValue(MakeIndexedValue(
    Entry(8, timeClient.GetTime()), "goodbye"), Sequence(2));
  dataStore.Store(valueD);
  vector<SequencedIndexedEntry> entries = dataStore.LoadAll();
  vector<SequencedIndexedEntry> expectedEntries =
    {valueA, valueB, valueC, valueD};
  CPPUNIT_ASSERT(entries.size() == expectedEntries.size());
  while(!expectedEntries.empty()) {
    auto expectedEntry = expectedEntries.back();
    CPPUNIT_ASSERT(std::find(entries.begin(), entries.end(), expectedEntry) !=
      entries.end());
    expectedEntries.pop_back();
  }
}
