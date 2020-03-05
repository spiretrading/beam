#include "Beam/QueriesTests/SessionCachedDataStoreTester.hpp"
#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SessionCachedDataStore.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::Threading;
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

  using BaseDataStore = LocalDataStore<BasicQuery<string>, Entry,
    EvaluatorTranslator<QueryTypes>>;
  using DataStore = SessionCachedDataStore<BaseDataStore*,
    EvaluatorTranslator<QueryTypes>>;
  using SequencedEntry = SequencedValue<Entry>;
  using SequencedIndexedEntry = SequencedValue<IndexedValue<Entry, string>>;

  Entry::Entry() {}

  Entry::Entry(int value, ptime timestamp)
      : m_value(value),
        m_timestamp(timestamp) {}

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }

  SequencedIndexedEntry StoreValue(DataStore& dataStore, string index,
      int value, const ptime& timestamp,
      const Beam::Queries::Sequence& sequence) {
    auto entry = SequencedValue(IndexedValue(Entry(value, timestamp), index),
      sequence);
    dataStore.Store(entry);
    return entry;
  }

  void TestQuery(DataStore& dataStore, string index,
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

void SessionCachedDataStoreTester::TestStoreAndLoad() {
  BaseDataStore baseDataStore;
  DataStore dataStore(&baseDataStore, 10);
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
    SnapshotLimit(SnapshotLimit::Type::HEAD, 0), std::vector<SequencedEntry>());
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 1), {entryA});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 2), {entryA, entryB});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 3), {entryA, entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::HEAD, 4), {entryA, entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 0), std::vector<SequencedEntry>());
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 2), {entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 3), {entryA, entryB, entryC});
  TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 4), {entryA, entryB, entryC});
}

void SessionCachedDataStoreTester::TestForwardCoherence() {
  BaseDataStore baseDataStore;
  DataStore dataStore(&baseDataStore, 10);
  IncrementalTimeClient timeClient;
  Beam::Queries::Sequence sequence(100);
  auto entryA = SequencedValue(IndexedValue(Entry(100, timeClient.GetTime()),
    "hello"), sequence);
  baseDataStore.Store(entryA);
  sequence = Increment(sequence);
  auto entryB = SequencedValue(IndexedValue(Entry(200, timeClient.GetTime()),
    "hello"), sequence);
  baseDataStore.Store(entryB);
  {
    BasicQuery<string> query;
    query.SetIndex("hello");
    query.SetRange(Sequence(100), Sequence(101));
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queryResult = dataStore.Load(query);
    CPPUNIT_ASSERT(queryResult.size() == 2);
    CPPUNIT_ASSERT(queryResult[0] == entryA);
    CPPUNIT_ASSERT(queryResult[1] == entryB);
  }
  for(auto i = Sequence(102); i < Sequence(130); i = Increment(i)) {
    auto entry = SequencedValue(IndexedValue(
      Entry(static_cast<int>(i.GetOrdinal()), timeClient.GetTime()), "hello"),
      i);
    dataStore.Store(entry);
  }
  {
    BasicQuery<string> query;
    query.SetIndex("hello");
    query.SetRange(Sequence(106), Sequence(107));
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queryResult = dataStore.Load(query);
    CPPUNIT_ASSERT(queryResult.size() == 2);
    CPPUNIT_ASSERT(queryResult[0].GetSequence().GetOrdinal() == 106);
    CPPUNIT_ASSERT(queryResult[1].GetSequence().GetOrdinal() == 107);
  }
  {
    BasicQuery<string> query;
    query.SetIndex("hello");
    query.SetRange(Sequence(104), Sequence(105));
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queryResult = dataStore.Load(query);
    CPPUNIT_ASSERT(queryResult.size() == 2);
    CPPUNIT_ASSERT(queryResult[0].GetSequence().GetOrdinal() == 104);
    CPPUNIT_ASSERT(queryResult[1].GetSequence().GetOrdinal() == 105);
  }
  {
    BasicQuery<string> query;
    query.SetIndex("hello");
    query.SetRange(Sequence(108), Sequence(115));
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queryResult = dataStore.Load(query);
    CPPUNIT_ASSERT(queryResult.size() == 8);
    CPPUNIT_ASSERT(queryResult[0].GetSequence().GetOrdinal() == 108);
    CPPUNIT_ASSERT(queryResult[7].GetSequence().GetOrdinal() == 115);
  }
}
