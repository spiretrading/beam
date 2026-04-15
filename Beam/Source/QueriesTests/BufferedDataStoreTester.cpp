#include <atomic>
#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/BufferedDataStore.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/QueriesTests/TestDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using TestLocalDataStore = LocalDataStore<
    BasicQuery<std::string>, TestEntry, EvaluatorTranslator<QueryTypes>>;
  using DataStore = BufferedDataStore<TestLocalDataStore>;
  using DataStoreDispatcher = TestDataStore<BasicQuery<std::string>, TestEntry>;
  using IntrusiveDataStore = BufferedDataStore<
    std::shared_ptr<DataStoreDispatcher>, EvaluatorTranslator<QueryTypes>>;
}

TEST_SUITE("BufferedDataStore") {
  TEST_CASE("store_and_load") {
    auto data_store = DataStore(init(), 10);
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(data_store, "hello", 100,
      time_from_string("2016-07-30 04:12:55:12"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(data_store, "hello", 200,
      time_from_string("2016-07-30 04:12:55:15"), sequence);
    sequence = increment(sequence);
    auto entry_c = store(data_store, "hello", 300,
      time_from_string("2016-07-30 04:12:55:17"), sequence);
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::UNLIMITED, {entry_a, entry_b, entry_c});
    test_query(
      data_store, "hello", Beam::Range::TOTAL, SnapshotLimit::from_head(0), {});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(1), {entry_a});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(2), {entry_a, entry_b});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(3), {entry_a, entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(4), {entry_a, entry_b, entry_c});
    test_query(
      data_store, "hello", Beam::Range::TOTAL, SnapshotLimit::from_tail(0), {});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(1), {entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(2), {entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(3), {entry_a, entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(4), {entry_a, entry_b, entry_c});
  }

  TEST_CASE("head_spanning_load") {
    auto local_data_store = TestLocalDataStore();
    auto data_store = BufferedDataStore(&local_data_store, 10);
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(local_data_store, "hello", 100,
      time_from_string("2016-07-30 04:12:55:20"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(local_data_store, "hello", 101,
      time_from_string("2016-07-30 04:12:55:25"), sequence);
    data_store.store(entry_b);
    sequence = increment(sequence);
    auto entry_c = store(data_store, "hello", 102,
      time_from_string("2016-07-30 04:12:55:30"), sequence);
    sequence = increment(sequence);
    auto entry_d = store(data_store, "hello", 103,
      time_from_string("2016-07-30 04:12:55:32"), sequence);
    sequence = increment(sequence);
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::UNLIMITED, {entry_a, entry_b, entry_c, entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(1), {entry_a});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(2), {entry_a, entry_b});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(3), {entry_a, entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(4), {entry_a, entry_b, entry_c, entry_d});
  }

  TEST_CASE("tail_spanning_load") {
    auto local_data_store = TestLocalDataStore();
    auto data_store = BufferedDataStore(&local_data_store, 10);
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(local_data_store, "hello", 100,
      time_from_string("2016-07-30 04:12:55:00"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(local_data_store, "hello", 101,
      time_from_string("2016-07-30 04:12:55:01"), sequence);
    data_store.store(entry_b);
    sequence = increment(sequence);
    auto entry_c = store(data_store, "hello", 102,
      time_from_string("2016-07-30 04:12:55:02"), sequence);
    sequence = increment(sequence);
    auto entry_d = store(data_store, "hello", 103,
      time_from_string("2016-07-30 04:12:55:03"), sequence);
    sequence = increment(sequence);
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(1), {entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(2), {entry_c, entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(3), {entry_b, entry_c, entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(4), {entry_a, entry_b, entry_c, entry_d});
  }

  TEST_CASE("flush_failure_retains_data") {
    auto dispatcher = std::make_shared<DataStoreDispatcher>();
    auto data_store = IntrusiveDataStore(dispatcher, 1);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<DataStoreDispatcher::Operation>>>();
    dispatcher->get_operation_publisher().monitor(operations);
    auto entry_a = SequencedIndexedTestEntry();
    {
      auto handler = RoutineHandler(spawn([&] {
        auto operation = operations->pop();
        auto& store_operation =
          std::get<DataStoreDispatcher::StoreOperation>(*operation);
        store_operation.m_result.set_exception(
          std::runtime_error("store failed"));
      }));
      auto sequence = Beam::Sequence(5);
      entry_a = store(data_store, "hello", 100,
        time_from_string("2016-07-30 04:12:55:12"), sequence);
    }
    {
      auto handler = RoutineHandler(spawn([&] {
        auto operation = operations->pop();
        auto& load_operation =
          std::get<DataStoreDispatcher::LoadOperation>(*operation);
        load_operation.m_result.set(std::vector<SequencedTestEntry>());
      }));
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::UNLIMITED, {entry_a});
    }
    auto drain = RoutineHandler(spawn([&] {
      try {
        while(true) {
          auto operation = operations->pop();
          if(auto store_op =
              std::get_if<DataStoreDispatcher::StoreOperation>(&*operation)) {
            store_op->m_result.set();
          } else {
            auto& load_op =
              std::get<DataStoreDispatcher::LoadOperation>(*operation);
            load_op.m_result.set(std::vector<SequencedTestEntry>());
          }
        }
      } catch(const PipeBrokenException&) {}
    }));
    data_store.close();
    operations->close();
  }

  TEST_CASE("flush_failure_retries_on_next_flush") {
    auto dispatcher = std::make_shared<DataStoreDispatcher>();
    auto data_store = IntrusiveDataStore(dispatcher, 1);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<DataStoreDispatcher::Operation>>>();
    dispatcher->get_operation_publisher().monitor(operations);
    auto sequence = Beam::Sequence(5);
    auto entry_a = SequencedIndexedTestEntry();
    auto entry_b = SequencedIndexedTestEntry();
    {
      auto handler = RoutineHandler(spawn([&] {
        auto operation = operations->pop();
        auto& store_operation =
          std::get<DataStoreDispatcher::StoreOperation>(*operation);
        store_operation.m_result.set_exception(
          std::runtime_error("store failed"));
      }));
      entry_a = store(data_store, "hello", 100,
        time_from_string("2016-07-30 04:12:55:12"), sequence);
    }
    auto stored_values = std::vector<SequencedIndexedTestEntry>();
    {
      auto handler = RoutineHandler(spawn([&] {
        auto operation = operations->pop();
        auto& store_operation =
          std::get<DataStoreDispatcher::StoreOperation>(*operation);
        stored_values = store_operation.m_values;
        store_operation.m_result.set();
      }));
      sequence = increment(sequence);
      entry_b = store(data_store, "hello", 200,
        time_from_string("2016-07-30 04:12:55:15"), sequence);
    }
    auto has_entry_a = std::ranges::find_if(stored_values,
      [&] (const auto& value) {
        return value.get_sequence() == entry_a.get_sequence();
      }) != stored_values.end();
    REQUIRE(has_entry_a);
    auto drain = RoutineHandler(spawn([&] {
      try {
        while(true) {
          auto operation = operations->pop();
          if(auto store_op =
              std::get_if<DataStoreDispatcher::StoreOperation>(
                &*operation)) {
            store_op->m_result.set();
          } else {
            auto& load_op =
              std::get<DataStoreDispatcher::LoadOperation>(*operation);
            load_op.m_result.set(std::vector<SequencedTestEntry>());
          }
        }
      } catch(const PipeBrokenException&) {}
    }));
    data_store.close();
    operations->close();
  }

  TEST_CASE("exception_handler") {
    auto dispatcher = std::make_shared<DataStoreDispatcher>();
    auto handler_called = std::atomic<bool>(false);
    auto exception_handler = IntrusiveDataStore::ExceptionHandler(
      [&] (const std::exception_ptr& e) {
        handler_called = true;
        try {
          std::rethrow_exception(e);
        } catch(const std::runtime_error& ex) {
          REQUIRE(std::string(ex.what()) == "store failed");
        }
      });
    auto data_store = IntrusiveDataStore(dispatcher, 1, exception_handler);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<DataStoreDispatcher::Operation>>>();
    dispatcher->get_operation_publisher().monitor(operations);
    {
      auto handler = RoutineHandler(spawn([&] {
        auto operation = operations->pop();
        auto& store_operation =
          std::get<DataStoreDispatcher::StoreOperation>(*operation);
        store_operation.m_result.set_exception(
          std::runtime_error("store failed"));
      }));
      auto sequence = Beam::Sequence(5);
      store(data_store, "hello", 100,
        time_from_string("2016-07-30 04:12:55:12"), sequence);
    }
    auto drain = RoutineHandler(spawn([&] {
      try {
        while(true) {
          auto operation = operations->pop();
          if(auto store_op =
              std::get_if<DataStoreDispatcher::StoreOperation>(
                &*operation)) {
            store_op->m_result.set();
          } else {
            auto& load_op =
              std::get<DataStoreDispatcher::LoadOperation>(*operation);
            load_op.m_result.set(std::vector<SequencedTestEntry>());
          }
        }
      } catch(const PipeBrokenException&) {}
    }));
    data_store.close();
    operations->close();
    REQUIRE(handler_called);
  }
}
