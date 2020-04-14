#include <doctest/doctest.h>
#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/CachedDataStore.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Threading;
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

  using BaseDataStore = LocalDataStore<BasicQuery<std::string>, Entry,
    EvaluatorTranslator<QueryTypes>>;
  using DataStore = CachedDataStore<BaseDataStore*,
    EvaluatorTranslator<QueryTypes>>;
  using SequencedEntry = SequencedValue<Entry>;
  using SequencedIndexedEntry =
    SequencedValue<IndexedValue<Entry, std::string>>;

  Entry::Entry(int value, ptime timestamp)
    : m_value(value),
      m_timestamp(timestamp) {}

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }

  std::ostream& operator <<(std::ostream& out, const Entry& entry) {
    return out << entry.m_value;
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

TEST_SUITE("CachedDataStore") {
  TEST_CASE("store_and_load") {
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

  TEST_CASE("forward_coherence") {
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
      BasicQuery<std::string> query;
      query.SetIndex("hello");
      query.SetRange(Beam::Queries::Sequence(100),
        Beam::Queries::Sequence(101));
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto queryResult = dataStore.Load(query);
      REQUIRE(queryResult.size() == 2);
      REQUIRE(queryResult[0] == entryA);
      REQUIRE(queryResult[1] == entryB);
    }
    for(auto i = Beam::Queries::Sequence(102); i < Beam::Queries::Sequence(130);
        i = Increment(i)) {
      auto entry = SequencedValue(IndexedValue(
        Entry(static_cast<int>(i.GetOrdinal()), timeClient.GetTime()), "hello"),
        i);
      dataStore.Store(entry);
    }
    {
      BasicQuery<std::string> query;
      query.SetIndex("hello");
      query.SetRange(Beam::Queries::Sequence(106),
        Beam::Queries::Sequence(107));
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto queryResult = dataStore.Load(query);
      REQUIRE(queryResult.size() == 2);
      REQUIRE(queryResult[0].GetSequence().GetOrdinal() == 106);
      REQUIRE(queryResult[1].GetSequence().GetOrdinal() == 107);
    }
    {
      BasicQuery<std::string> query;
      query.SetIndex("hello");
      query.SetRange(Beam::Queries::Sequence(104),
        Beam::Queries::Sequence(105));
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto queryResult = dataStore.Load(query);
      REQUIRE(queryResult.size() == 2);
      REQUIRE(queryResult[0].GetSequence().GetOrdinal() == 104);
      REQUIRE(queryResult[1].GetSequence().GetOrdinal() == 105);
    }
    {
      BasicQuery<std::string> query;
      query.SetIndex("hello");
      query.SetRange(Beam::Queries::Sequence(108),
        Beam::Queries::Sequence(115));
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto queryResult = dataStore.Load(query);
      REQUIRE(queryResult.size() == 8);
      REQUIRE(queryResult[0].GetSequence().GetOrdinal() == 108);
      REQUIRE(queryResult[7].GetSequence().GetOrdinal() == 115);
    }
  }

  TEST_CASE("backward_coherence") {
    BaseDataStore baseDataStore;
    DataStore dataStore(&baseDataStore, 10);
    IncrementalTimeClient timeClient;
    Beam::Queries::Sequence sequence(108);
    auto entryA = SequencedValue(IndexedValue(Entry(100, timeClient.GetTime()),
      "hello"), sequence);
    baseDataStore.Store(entryA);
    sequence = Increment(sequence);
    auto entryB = SequencedValue(IndexedValue(Entry(200, timeClient.GetTime()),
      "hello"), sequence);
    baseDataStore.Store(entryB);
    {
      BasicQuery<std::string> query;
      query.SetIndex("hello");
      query.SetRange(Beam::Queries::Sequence(108),
        Beam::Queries::Sequence(109));
      query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 2);
      auto queryResult = dataStore.Load(query);
      REQUIRE(queryResult.size() == 2);
      REQUIRE(queryResult[0] == entryA);
      REQUIRE(queryResult[1] == entryB);
    }
    for(auto i = Beam::Queries::Sequence(90); i < Beam::Queries::Sequence(108);
        i = Increment(i)) {
      auto entry = SequencedValue(IndexedValue(
        Entry(static_cast<int>(i.GetOrdinal()), timeClient.GetTime()), "hello"),
        i);
      dataStore.Store(entry);
    }
    {
      BasicQuery<std::string> query;
      query.SetIndex("hello");
      query.SetRange(Beam::Queries::Sequence(106),
        Beam::Queries::Sequence(107));
      query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 2);
      auto queryResult = dataStore.Load(query);
      REQUIRE(queryResult.size() == 2);
      REQUIRE(queryResult[0].GetSequence() == Beam::Queries::Sequence(106));
      REQUIRE(queryResult[1].GetSequence() == Beam::Queries::Sequence(107));
    }
    {
      BasicQuery<std::string> query;
      query.SetIndex("hello");
      query.SetRange(Beam::Queries::Sequence(95),
        Beam::Queries::Sequence(105));
      query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 7);
      auto queryResult = dataStore.Load(query);
      REQUIRE(queryResult.size() == 7);
      REQUIRE(queryResult.front().GetSequence().GetOrdinal() == 99);
      REQUIRE(queryResult.back().GetSequence().GetOrdinal() == 105);
    }
  }
}
