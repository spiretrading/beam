#include <boost/date_time/posix_time/ptime.hpp>
#include <doctest/doctest.h>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/QueriesTests/TestDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace {
  using DataStore = TestDataStore<BasicQuery<std::string>, TestEntry>;
}

TEST_SUITE("TestDataStore") {
  TEST_CASE("store") {
    auto data_store = DataStore();
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    data_store.get_operation_publisher().monitor(operations);
    auto result = Async<void>();
    auto store_routine = RoutineHandler(spawn([&] {
      try {
        data_store.store(SequencedValue(IndexedValue(
          TestEntry(123, ptime(date(2018, 5, 3))), std::string("hello")),
          Beam::Sequence(110)));
        result.get_eval().set();
      } catch(const std::exception&) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    auto operation = operations->pop();
    auto store_operation = std::get_if<DataStore::StoreOperation>(&*operation);
    REQUIRE(store_operation);
    store_operation->m_result.set();
    REQUIRE_NOTHROW(result.get());
  }

  TEST_CASE("store_exception") {
    auto data_store = DataStore();
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    data_store.get_operation_publisher().monitor(operations);
    auto result = Async<void>();
    auto store_routine = RoutineHandler(spawn([&] {
      try {
        data_store.store(SequencedValue(IndexedValue(
          TestEntry(123, ptime(date(2018, 5, 3))), std::string("hello")),
          Beam::Sequence(110)));
        result.get_eval().set();
      } catch(const std::exception&) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    auto operation = operations->pop();
    auto store_operation = std::get_if<DataStore::StoreOperation>(&*operation);
    REQUIRE(store_operation);
    store_operation->m_result.set_exception(
      std::runtime_error("Store failed."));
    REQUIRE_THROWS_AS(result.get(), std::runtime_error);
  }

  TEST_CASE("load") {
    auto data_store = DataStore();
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    data_store.get_operation_publisher().monitor(operations);
    auto result = Async<std::vector<SequencedTestEntry>>();
    auto query = BasicQuery<std::string>();
    query.set_index("index");
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto loadRoutine = RoutineHandler(spawn([&] {
      try {
        result.get_eval().set(data_store.load(query));
      } catch(const std::exception&) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    auto operation = operations->pop();
    auto load_operation = std::get_if<DataStore::LoadOperation>(&*operation);
    REQUIRE(load_operation);
    auto series = std::vector<SequencedTestEntry>();
    for(auto i = 0; i < 10; ++i) {
      series.push_back(SequencedTestEntry(
        TestEntry(i, ptime(date(2018, i % 13 + 1, 3))),
        Beam::Sequence(i + 100)));
    }
    load_operation->m_result.set(series);
    REQUIRE(result.get() == series);
  }

  TEST_CASE("load_exception") {
    auto data_store = DataStore();
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    data_store.get_operation_publisher().monitor(operations);
    auto result = Async<std::vector<SequencedTestEntry>>();
    auto query = BasicQuery<std::string>();
    query.set_index("index");
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto loadRoutine = RoutineHandler(spawn([&] {
      try {
        result.get_eval().set(data_store.load(query));
      } catch(const std::exception&) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    auto operation = operations->pop();
    auto load_operation = std::get_if<DataStore::LoadOperation>(&*operation);
    REQUIRE(load_operation);
    load_operation->m_result.set_exception(std::runtime_error("Load failed."));
    REQUIRE_THROWS_AS(result.get(), std::runtime_error);
  }
}
