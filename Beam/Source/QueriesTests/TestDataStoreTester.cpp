#include <boost/date_time/posix_time/ptime.hpp>
#include <doctest/doctest.h>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/QueriesTests/TestDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace {
  using DataStore = TestDataStore<BasicQuery<std::string>, TestEntry>;

  void Open(DataStore& dataStore) {
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    auto result = Async<void>();
    auto openRoutine = RoutineHandler(Spawn(
      [&] {
        try {
          dataStore.Open();
          result.GetEval().SetResult();
        } catch(const std::exception&) {
          result.GetEval().SetException(std::current_exception());
        }
      }));
    auto operation = operations->Pop();
    auto openOperation = std::get_if<DataStore::OpenOperation>(&*operation);
    REQUIRE(openOperation);
    openOperation->m_result.SetResult();
    REQUIRE_NOTHROW(result.Get());
  }
}

TEST_SUITE("TestDataStore") {
  TEST_CASE("open_exception") {
    auto dataStore = DataStore();
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    auto result = Async<void>();
    auto openRoutine = RoutineHandler(Spawn(
      [&] {
        try {
          dataStore.Open();
          result.GetEval().SetResult();
        } catch(const std::exception&) {
          result.GetEval().SetException(std::current_exception());
        }
      }));
    auto operation = operations->Pop();
    auto openOperation = std::get_if<DataStore::OpenOperation>(&*operation);
    REQUIRE(openOperation);
    openOperation->m_result.SetException(ConnectException());
    REQUIRE_THROWS_AS(result.Get(), ConnectException);
  }

  TEST_CASE("store") {
    auto dataStore = DataStore();
    Open(dataStore);
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    auto result = Async<void>();
    auto storeRoutine = RoutineHandler(Spawn(
      [&] {
        try {
          dataStore.Store(SequencedValue(IndexedValue(
            TestEntry{123, ptime(date(2018, 5, 3))}, std::string("hello")),
            Beam::Queries::Sequence(110)));
          result.GetEval().SetResult();
        } catch(const std::exception&) {
          result.GetEval().SetException(std::current_exception());
        }
      }));
    auto operation = operations->Pop();
    auto storeOperation = std::get_if<DataStore::StoreOperation>(&*operation);
    REQUIRE(storeOperation);
    storeOperation->m_result.SetResult();
    REQUIRE_NOTHROW(result.Get());
  }

  TEST_CASE("store_exception") {
    auto dataStore = DataStore();
    Open(dataStore);
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    auto result = Async<void>();
    auto storeRoutine = RoutineHandler(Spawn(
      [&] {
        try {
          dataStore.Store(SequencedValue(IndexedValue(
            TestEntry{123, ptime(date(2018, 5, 3))}, std::string("hello")),
            Beam::Queries::Sequence(110)));
          result.GetEval().SetResult();
        } catch(const std::exception&) {
          result.GetEval().SetException(std::current_exception());
        }
      }));
    auto operation = operations->Pop();
    auto storeOperation = std::get_if<DataStore::StoreOperation>(&*operation);
    REQUIRE(storeOperation);
    storeOperation->m_result.SetException(std::runtime_error("Store failed."));
    REQUIRE_THROWS_AS(result.Get(), std::runtime_error);
  }

  TEST_CASE("load") {
    auto dataStore = DataStore();
    Open(dataStore);
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    auto result = Async<std::vector<SequencedTestEntry>>();
    auto query = BasicQuery<std::string>();
    query.SetIndex("index");
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto loadRoutine = RoutineHandler(Spawn(
      [&] {
        try {
          result.GetEval().SetResult(dataStore.Load(query));
        } catch(const std::exception&) {
          result.GetEval().SetException(std::current_exception());
        }
      }));
    auto operation = operations->Pop();
    auto loadOperation = std::get_if<DataStore::LoadOperation>(&*operation);
    REQUIRE(loadOperation);
    auto series = std::vector<SequencedTestEntry>();
    for(auto i = 0; i < 10; ++i) {
      series.push_back(SequencedTestEntry(
        TestEntry{i, ptime(date(2018, i % 13 + 1, 3))},
        Beam::Queries::Sequence(i + 100)));
    }
    loadOperation->m_result.SetResult(series);
    REQUIRE(result.Get() == series);
  }

  TEST_CASE("load_exception") {
    auto dataStore = DataStore();
    Open(dataStore);
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    auto result = Async<std::vector<SequencedTestEntry>>();
    auto query = BasicQuery<std::string>();
    query.SetIndex("index");
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto loadRoutine = RoutineHandler(Spawn(
      [&] {
        try {
          result.GetEval().SetResult(dataStore.Load(query));
        } catch(const std::exception&) {
          result.GetEval().SetException(std::current_exception());
        }
      }));
    auto operation = operations->Pop();
    auto loadOperation = std::get_if<DataStore::LoadOperation>(&*operation);
    REQUIRE(loadOperation);
    loadOperation->m_result.SetException(std::runtime_error("Load failed."));
    REQUIRE_THROWS_AS(result.Get(), std::runtime_error);
  }
}
