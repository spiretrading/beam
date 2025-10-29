#include <algorithm>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/AsyncDataStore.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
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
  using DataStore = AsyncDataStore<TestLocalDataStore>;
  using DataStoreDispatcher = TestDataStore<BasicQuery<std::string>, TestEntry>;
  using IntrusiveDataStore = AsyncDataStore<
    std::shared_ptr<DataStoreDispatcher>, EvaluatorTranslator<QueryTypes>>;
}

TEST_SUITE("AsyncDataStore") {
  TEST_CASE("store_and_load") {
    auto data_store = DataStore(init());
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(data_store, "hello", 100,
      time_from_string("2016-07-30 04:12:55:12"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(data_store, "hello", 200,
      time_from_string("2016-07-30 04:12:55:15"), sequence);
    sequence = increment(sequence);
    auto entry_c = store(data_store, "hello", 300,
      time_from_string("2016-07-30 04:12:55:18"), sequence);
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::UNLIMITED, {entry_a, entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(0), {});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(1), {entry_a});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(2), {entry_a, entry_b});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(3), {entry_a, entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(4), {entry_a, entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(0), {});
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
    auto data_store = AsyncDataStore(&local_data_store);
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(local_data_store, "hello", 100,
      time_from_string("2016-07-30 04:12:55:00"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(local_data_store, "hello", 101,
      time_from_string("2016-07-30 04:12:55:03"), sequence);
    sequence = increment(sequence);
    auto entry_c = store(data_store, "hello", 102,
      time_from_string("2016-07-30 04:12:55:06"), sequence);
    sequence = increment(sequence);
    auto entry_d = store(data_store, "hello", 103,
      time_from_string("2016-07-30 04:12:55:07"), sequence);
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(1), {entry_a});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(2), {entry_a, entry_b});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(3), {entry_a, entry_b, entry_c});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_head(4), {entry_a, entry_b, entry_c, entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::UNLIMITED, {entry_a, entry_b, entry_c, entry_d});
  }

  TEST_CASE("tail_spanning_load") {
    auto local_data_store = TestLocalDataStore();
    auto data_store = AsyncDataStore(&local_data_store);
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(local_data_store, "hello", 100,
      time_from_string("2016-07-30 04:12:55:44"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(local_data_store, "hello", 101,
      time_from_string("2016-07-30 04:12:55:48"), sequence);
    sequence = increment(sequence);
    auto entry_c = store(data_store, "hello", 102,
      time_from_string("2016-07-30 04:12:55:50"), sequence);
    sequence = increment(sequence);
    auto entry_d = store(data_store, "hello", 103,
      time_from_string("2016-07-30 04:12:55:51"), sequence);
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(1), {entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(2), {entry_c, entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(3), {entry_b, entry_c, entry_d});
    test_query(data_store, "hello", Beam::Range::TOTAL,
      SnapshotLimit::from_tail(4),
      {entry_a, entry_b, entry_c, entry_d});
  }

  TEST_CASE("buffered_load") {
    auto dispatcher = std::make_shared<DataStoreDispatcher>();
    auto data_store = IntrusiveDataStore(dispatcher);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<DataStoreDispatcher::Operation>>>();
    dispatcher->get_operation_publisher().monitor(operations);
    auto entry_a = SequencedIndexedTestEntry();
    auto entry_b = SequencedIndexedTestEntry();
    auto entry_c = SequencedIndexedTestEntry();
    auto first_store_operation =
      std::optional<DataStoreDispatcher::StoreOperation*>();
    {
      auto handler = RoutineHandler(spawn([&] {
        auto operation = operations->pop();
        auto store_operation =
          std::get_if<DataStoreDispatcher::StoreOperation>(&*operation);
        first_store_operation = store_operation;
      }));
      auto sequence = Beam::Sequence(5);
      entry_a = store(data_store, "hello", 100,
        time_from_string("2016-07-30 04:12:55:21"), sequence);
      sequence = increment(sequence);
      entry_b = store(data_store, "hello", 200,
        time_from_string("2016-07-30 04:12:55:22"), sequence);
      sequence = increment(sequence);
      entry_c = store(data_store, "hello", 300,
        time_from_string("2016-07-30 04:12:55:23"), sequence);
    }
    {
      auto handler = RoutineHandler(spawn([&] {
        for(auto i = 0; i != 11; ++i) {
          auto operation = operations->pop();
          auto& load_operation =
            std::get<DataStoreDispatcher::LoadOperation>(*operation);
          load_operation.m_result.set(std::vector<SequencedTestEntry>());
        }
      }));
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::UNLIMITED, {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(0), {});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(1), {entry_a});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(2), {entry_a, entry_b});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(3), {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(4), {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(0), {});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(1), {entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(2), {entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(3), {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(4), {entry_a, entry_b, entry_c});
    }
    REQUIRE(first_store_operation.has_value());
    auto entries_left = 3 - (*first_store_operation)->m_values.size();
    (*first_store_operation)->m_result.set();
    while(entries_left != 0) {
      auto operation = operations->pop();
      auto& store_operation =
        std::get<DataStoreDispatcher::StoreOperation>(*operation);
      entries_left -= store_operation.m_values.size();
      store_operation.m_result.set();
    }
  }

  TEST_CASE("flushed_load") {
    auto local_data_store = TestLocalDataStore();
    auto dispatcher = std::make_shared<DataStoreDispatcher>();
    auto data_store = IntrusiveDataStore(dispatcher);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<DataStoreDispatcher::Operation>>>();
    dispatcher->get_operation_publisher().monitor(operations);
    auto entry_a = SequencedIndexedTestEntry();
    auto entry_b = SequencedIndexedTestEntry();
    auto entry_c = SequencedIndexedTestEntry();
    {
      auto handler = RoutineHandler(spawn([&] {
        for(auto i = 3; i != 0;) {
          auto operation = operations->pop();
          auto& store_operation =
            std::get<DataStoreDispatcher::StoreOperation>(*operation);
          i -= store_operation.m_values.size();
          local_data_store.store(store_operation.m_values);
          store_operation.m_result.set();
        }
      }));
      auto sequence = Beam::Sequence(5);
      entry_a = store(data_store, "hello", 100,
        time_from_string("2016-07-30 04:12:55:12"), sequence);
      sequence = increment(sequence);
      entry_b = store(data_store, "hello", 200,
        time_from_string("2016-07-30 04:12:55:15"), sequence);
      sequence = increment(sequence);
      entry_c = store(data_store, "hello", 300,
        time_from_string("2016-07-30 04:12:55:18"), sequence);
    }
    {
      auto handler = RoutineHandler(spawn([&] {
        for(auto i = 0; i != 11; ++i) {
          auto operation = operations->pop();
          auto& load_operation =
            std::get<DataStoreDispatcher::LoadOperation>(*operation);
          auto values = local_data_store.load(load_operation.m_query);
          load_operation.m_result.set(values);
        }
      }));
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::UNLIMITED, {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(0), {});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(1), {entry_a});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(2), {entry_a, entry_b});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(3), {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_head(4), {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(0), {});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(1), {entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(2), {entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(3), {entry_a, entry_b, entry_c});
      test_query(data_store, "hello", Beam::Range::TOTAL,
        SnapshotLimit::from_tail(4), {entry_a, entry_b, entry_c});
    }
  }
}
