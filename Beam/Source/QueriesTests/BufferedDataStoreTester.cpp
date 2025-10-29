#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/BufferedDataStore.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using TestLocalDataStore = LocalDataStore<
    BasicQuery<std::string>, TestEntry, EvaluatorTranslator<QueryTypes>>;
  using DataStore = BufferedDataStore<TestLocalDataStore>;
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
}
