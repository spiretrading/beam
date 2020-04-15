#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/CachedDataStore.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;

namespace {
  using BaseDataStore = LocalDataStore<BasicQuery<std::string>, TestEntry,
    EvaluatorTranslator<QueryTypes>>;
  using DataStore = CachedDataStore<BaseDataStore*,
    EvaluatorTranslator<QueryTypes>>;
}

TEST_SUITE("CachedDataStore") {
  TEST_CASE("store_and_load") {
    auto baseDataStore = BaseDataStore();
    auto dataStore = DataStore(&baseDataStore, 10);
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

  TEST_CASE("forward_coherence") {
    auto baseDataStore = BaseDataStore();
    auto dataStore = DataStore(&baseDataStore, 10);
    auto timeClient = IncrementalTimeClient();
    auto sequence = Beam::Queries::Sequence(100);
    auto entryA = SequencedValue(IndexedValue(
      TestEntry{100, timeClient.GetTime()}, "hello"), sequence);
    baseDataStore.Store(entryA);
    sequence = Increment(sequence);
    auto entryB = SequencedValue(IndexedValue(
      TestEntry{200, timeClient.GetTime()}, "hello"), sequence);
    baseDataStore.Store(entryB);
    {
      auto query = BasicQuery<std::string>();
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
      auto entry = SequencedValue(IndexedValue(TestEntry{
        static_cast<int>(i.GetOrdinal()), timeClient.GetTime()}, "hello"), i);
      dataStore.Store(entry);
    }
    {
      auto query = BasicQuery<std::string>();
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
      auto query = BasicQuery<std::string>();
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
      auto query = BasicQuery<std::string>();
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
    auto baseDataStore = BaseDataStore();
    auto dataStore = DataStore(&baseDataStore, 10);
    auto timeClient = IncrementalTimeClient();
    auto sequence = Beam::Queries::Sequence(108);
    auto entryA = SequencedValue(IndexedValue(
      TestEntry{100, timeClient.GetTime()}, "hello"), sequence);
    baseDataStore.Store(entryA);
    sequence = Increment(sequence);
    auto entryB = SequencedValue(IndexedValue(
      TestEntry{200, timeClient.GetTime()}, "hello"), sequence);
    baseDataStore.Store(entryB);
    {
      auto query = BasicQuery<std::string>();
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
        TestEntry{static_cast<int>(i.GetOrdinal()), timeClient.GetTime()},
        "hello"), i);
      dataStore.Store(entry);
    }
    {
      auto query = BasicQuery<std::string>();
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
      auto query = BasicQuery<std::string>();
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
