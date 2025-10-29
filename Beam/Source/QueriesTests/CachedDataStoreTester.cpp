#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/CachedDataStore.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using BaseDataStore = LocalDataStore<
    BasicQuery<std::string>, TestEntry, EvaluatorTranslator<QueryTypes>>;
  using DataStore =
    CachedDataStore<BaseDataStore*, EvaluatorTranslator<QueryTypes>>;
}

TEST_SUITE("CachedDataStore") {
  TEST_CASE("store_and_load") {
    auto base_data_store = BaseDataStore();
    auto data_store = DataStore(&base_data_store, 10);
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(data_store, "hello", 100,
      time_from_string("2016-07-30 04:12:55:15"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(data_store, "hello", 200,
      time_from_string("2016-07-30 04:12:55:16"), sequence);
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

  TEST_CASE("forward_coherence") {
    auto base_data_store = BaseDataStore();
    auto data_store = DataStore(&base_data_store, 10);
    auto sequence = Beam::Sequence(100);
    auto entry_a = SequencedValue(
      IndexedValue(TestEntry(100, time_from_string("2016-07-30 04:12:55:17")),
        std::string("hello")), sequence);
    base_data_store.store(entry_a);
    sequence = increment(sequence);
    auto entry_b = SequencedValue(
      IndexedValue(TestEntry(200, time_from_string("2016-07-30 04:12:55:19")),
        std::string("hello")), sequence);
    base_data_store.store(entry_b);
    {
      auto query = BasicQuery<std::string>();
      query.set_index("hello");
      query.set_range(Beam::Sequence(100), Beam::Sequence(101));
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto result = data_store.load(query);
      REQUIRE(result.size() == 2);
      REQUIRE(result[0].get_value() == entry_a.get_value());
      REQUIRE(result[0].get_sequence() == entry_a.get_sequence());
      REQUIRE(result[1].get_value() == entry_b.get_value());
      REQUIRE(result[1].get_sequence() == entry_b.get_sequence());
    }
    auto timestamp = time_from_string("2016-07-30 04:12:55:22");
    for(auto i = Beam::Sequence(102); i < Beam::Sequence(130);
        i = increment(i)) {
      auto entry = SequencedValue(IndexedValue(TestEntry(
        static_cast<int>(i.get_ordinal()),
        timestamp + seconds(i.get_ordinal() - 102)), std::string("hello")), i);
      data_store.store(entry);
    }
    {
      auto query = BasicQuery<std::string>();
      query.set_index("hello");
      query.set_range(Beam::Sequence(106), Beam::Sequence(107));
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto result = data_store.load(query);
      REQUIRE(result.size() == 2);
      REQUIRE(result[0].get_sequence().get_ordinal() == 106);
      REQUIRE(result[1].get_sequence().get_ordinal() == 107);
    }
    {
      auto query = BasicQuery<std::string>();
      query.set_index("hello");
      query.set_range(Beam::Sequence(104), Beam::Sequence(105));
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto result = data_store.load(query);
      REQUIRE(result.size() == 2);
      REQUIRE(result[0].get_sequence().get_ordinal() == 104);
      REQUIRE(result[1].get_sequence().get_ordinal() == 105);
    }
    {
      auto query = BasicQuery<std::string>();
      query.set_index("hello");
      query.set_range(Beam::Sequence(108), Beam::Sequence(115));
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto result = data_store.load(query);
      REQUIRE(result.size() == 8);
      REQUIRE(result[0].get_sequence().get_ordinal() == 108);
      REQUIRE(result[7].get_sequence().get_ordinal() == 115);
    }
  }

  TEST_CASE("backward_coherence") {
    auto base_data_store = BaseDataStore();
    auto data_store = DataStore(&base_data_store, 10);
    auto sequence = Beam::Sequence(108);
    auto entry_a = SequencedValue(
      IndexedValue(TestEntry(100, time_from_string("2016-07-30 04:12:55:17")),
        std::string("hello")), sequence);
    base_data_store.store(entry_a);
    sequence = increment(sequence);
    auto entry_b = SequencedValue(
      IndexedValue(TestEntry(200, time_from_string("2016-07-30 04:12:55:19")),
      std::string("hello")), sequence);
    base_data_store.store(entry_b);
    {
      auto query = BasicQuery<std::string>();
      query.set_index("hello");
      query.set_range(Beam::Sequence(108), Beam::Sequence(109));
      query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 2);
      auto result = data_store.load(query);
      REQUIRE(result.size() == 2);
      REQUIRE(result[0].get_value() == entry_a.get_value());
      REQUIRE(result[0].get_sequence() == entry_a.get_sequence());
      REQUIRE(result[1].get_value() == entry_b.get_value());
      REQUIRE(result[1].get_sequence() == entry_b.get_sequence());
    }
    auto timestamp = time_from_string("2016-07-30 04:12:55:22");
    for(auto i = Beam::Sequence(90); i < Beam::Sequence(108);
        i = increment(i)) {
      auto entry = SequencedValue(
        IndexedValue(TestEntry(static_cast<int>(i.get_ordinal()),
          timestamp + seconds(i.get_ordinal() - 90)),
          std::string("hello")), i);
      data_store.store(entry);
    }
    {
      auto query = BasicQuery<std::string>();
      query.set_index("hello");
      query.set_range(Beam::Sequence(106), Beam::Sequence(107));
      query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 2);
      auto result = data_store.load(query);
      REQUIRE(result.size() == 2);
      REQUIRE(result[0].get_sequence() == Beam::Sequence(106));
      REQUIRE(result[1].get_sequence() == Beam::Sequence(107));
    }
    {
      auto query = BasicQuery<std::string>();
      query.set_index("hello");
      query.set_range(Beam::Sequence(95), Beam::Sequence(105));
      query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 7);
      auto result = data_store.load(query);
      REQUIRE(result.size() == 7);
      REQUIRE(result.front().get_sequence().get_ordinal() == 99);
      REQUIRE(result.back().get_sequence().get_ordinal() == 105);
    }
  }
}
