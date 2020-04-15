#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/BufferedDataStore.hpp"
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
  using TestLocalDataStore = LocalDataStore<BasicQuery<std::string>, TestEntry,
    EvaluatorTranslator<QueryTypes>>;
  using DataStore = BufferedDataStore<TestLocalDataStore>;
}

TEST_SUITE("BufferedDataStore") {
  TEST_CASE("store_and_load") {
    auto dataStore = DataStore(Initialize(), 10);
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

  TEST_CASE("head_spanning_load") {
    auto localDataStore = TestLocalDataStore();
    auto dataStore = BufferedDataStore<TestLocalDataStore*>(
      &localDataStore, 10);
    auto timeClient = IncrementalTimeClient();
    auto sequence = Beam::Queries::Sequence(5);
    auto entryA = StoreValue(localDataStore, "hello", 100, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    auto entryB = StoreValue(localDataStore, "hello", 101, timeClient.GetTime(),
      sequence);
    dataStore.Store(entryB);
    sequence = Increment(sequence);
    auto entryC = StoreValue(dataStore, "hello", 102, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    auto entryD = StoreValue(dataStore, "hello", 103, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit::Unlimited(), {entryA, entryB, entryC, entryD});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 1), {entryA});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 2), {entryA, entryB});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 3), {entryA, entryB, entryC});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 4),
      {entryA, entryB, entryC, entryD});
  }

  TEST_CASE("tail_spanning_load") {
    auto localDataStore = TestLocalDataStore();
    auto dataStore = BufferedDataStore<TestLocalDataStore*>(
      &localDataStore, 10);
    auto timeClient = IncrementalTimeClient();
    auto sequence = Beam::Queries::Sequence(5);
    auto entryA = StoreValue(localDataStore, "hello", 100, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    auto entryB = StoreValue(localDataStore, "hello", 101, timeClient.GetTime(),
      sequence);
    dataStore.Store(entryB);
    sequence = Increment(sequence);
    auto entryC = StoreValue(dataStore, "hello", 102, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    auto entryD = StoreValue(dataStore, "hello", 103, timeClient.GetTime(),
      sequence);
    sequence = Increment(sequence);
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryD});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 2), {entryC, entryD});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 3), {entryB, entryC, entryD});
    TestQuery(dataStore, "hello", Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 4),
      {entryA, entryB, entryC, entryD});
  }
}
