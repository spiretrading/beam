#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::TimeService;

namespace {
  using DataStore = LocalDataStore<BasicQuery<std::string>, TestEntry,
    EvaluatorTranslator<QueryTypes>>;
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
    auto valueA = SequencedValue(IndexedValue(
      TestEntry{5, timeClient.GetTime()}, "hello"), Beam::Queries::Sequence(1));
    dataStore.Store(valueA);
    auto valueB = SequencedValue(IndexedValue(
      TestEntry{6, timeClient.GetTime()}, "hello"), Beam::Queries::Sequence(2));
    dataStore.Store(valueB);
    auto valueC = SequencedValue(IndexedValue(
      TestEntry{7, timeClient.GetTime()}, "goodbye"),
      Beam::Queries::Sequence(1));
    dataStore.Store(valueC);
    auto valueD = SequencedValue(IndexedValue(
      TestEntry{8, timeClient.GetTime()}, "goodbye"),
      Beam::Queries::Sequence(2));
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
