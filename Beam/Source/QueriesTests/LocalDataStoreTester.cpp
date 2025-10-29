#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::mp11;
using namespace boost::posix_time;

namespace {
  using DataStore = LocalDataStore<
    BasicQuery<std::string>, TestEntry, EvaluatorTranslator<QueryTypes>>;

  struct CustomQueryTypes {
    using NativeTypes = mp_push_back<QueryTypes::NativeTypes, TestEntry>;
    using ValueTypes = mp_push_back<QueryTypes::ValueTypes, TestEntry>;
    using ComparableTypes = QueryTypes::ComparableTypes;
  };

  struct CustomTranslator : EvaluatorTranslator<CustomQueryTypes> {
    std::vector<int> m_special_values;

    CustomTranslator(std::vector<int> special_values)
      : m_special_values(std::move(special_values)) {}

    std::unique_ptr<EvaluatorTranslator<CustomQueryTypes>>
        make_translator() const override {
      return std::make_unique<CustomTranslator>(m_special_values);
    }

    void visit(const MemberAccessExpression& expression) override {
      expression.get_expression().apply(*this);
      auto value =
        static_pointer_cast<EvaluatorNode<TestEntry>>(get_evaluator());
      if(expression.get_name() == "is_special") {
        set_evaluator(make_function_evaluator_node(
          [special_values = m_special_values] (const TestEntry& entry) {
            return std::ranges::any_of(
              special_values, [&] (auto special_value) {
                return entry.m_value == special_value;
              });
        }, std::move(value)));
      } else {
        EvaluatorTranslator<CustomQueryTypes>::visit(expression);
      }
    }
  };
}

TEST_SUITE("LocalDataStore") {
  TEST_CASE("store_and_load") {
    auto data_store = DataStore();
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(data_store, "hello", 100,
      time_from_string("2024-05-06 13:21:53:06"), sequence);
    sequence = increment(sequence);
    auto entry_b = store(data_store, "hello", 200,
      time_from_string("2024-05-06 13:21:53:08"), sequence);
    sequence = increment(sequence);
    auto entry_c = store(data_store, "hello", 300,
      time_from_string("2024-05-06 13:21:53:10"), sequence);
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

  TEST_CASE("load_all") {
    auto data_store = DataStore();
    auto value_a = SequencedValue(
      IndexedValue(TestEntry(5, time_from_string("2024-05-06 13:21:53:06")),
        std::string("hello")), Beam::Sequence(1));
    data_store.store(value_a);
    auto value_b = SequencedValue(
      IndexedValue(TestEntry(6, time_from_string("2024-05-06 13:21:53:08")),
        std::string("hello")), Beam::Sequence(2));
    data_store.store(value_b);
    auto value_c = SequencedValue(
      IndexedValue(TestEntry(7, time_from_string("2024-05-06 13:21:53:09")),
        std::string("goodbye")), Beam::Sequence(1));
    data_store.store(value_c);
    auto value_d = SequencedValue(
      IndexedValue(TestEntry(8, time_from_string("2024-05-06 13:21:53:10")),
        std::string("goodbye")), Beam::Sequence(2));
    data_store.store(value_d);
    auto entries = data_store.load_all();
    auto expected_entries = std::vector{value_a, value_b, value_c, value_d};
    REQUIRE(entries.size() == expected_entries.size());
    while(!expected_entries.empty()) {
      auto expected_entry = expected_entries.back();
      REQUIRE(std::ranges::find(entries, expected_entry) != entries.end());
      expected_entries.pop_back();
    }
  }

  TEST_CASE("custom_translator") {
    auto special_values = std::vector{3, 5, 9};
    auto data_store = LocalDataStore<
      BasicQuery<std::string>, TestEntry, CustomTranslator>(special_values);
    auto timestamp = time_from_string("2024-05-06 13:21:53:00");
    for(auto i = 1; i <= 10; ++i) {
      auto value = SequencedValue(IndexedValue(
        TestEntry(i, timestamp + seconds(i)), std::string("hello")),
        Beam::Sequence(i));
      data_store.store(value);
    }
    auto special_query = BasicQuery<std::string>();
    special_query.set_index("hello");
    special_query.set_range(Range::HISTORICAL);
    special_query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    special_query.set_filter(MemberAccessExpression(
      "is_special", typeid(bool), ParameterExpression(0, typeid(TestEntry))));
    auto special_entries = data_store.load(special_query);
    REQUIRE(special_entries.size() == 3);
    REQUIRE(special_entries[0]->m_value == 3);
    REQUIRE(special_entries[1]->m_value == 5);
    REQUIRE(special_entries[2]->m_value == 9);
  }
}
