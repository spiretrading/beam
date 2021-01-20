#include <vector>
#include <boost/mpl/push_back.hpp>
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

  struct CustomQueryTypes {
    using NativeTypes = boost::mpl::insert<QueryTypes::NativeTypes,
      boost::mpl::end<QueryTypes::NativeTypes>::type, TestEntry>::type;
    using ValueTypes = boost::mpl::insert<QueryTypes::ValueTypes,
      boost::mpl::end<QueryTypes::NativeTypes>::type, TestEntry>::type;
    using ComparableTypes = QueryTypes::ComparableTypes;
  };

  struct CustomTranslator : EvaluatorTranslator<CustomQueryTypes> {
    std::vector<int> m_specialValues;

    CustomTranslator(const std::vector<int>& specialValues)
      : m_specialValues(std::move(specialValues)) {}

    std::unique_ptr<EvaluatorTranslator<CustomQueryTypes>>
        NewTranslator() const override {
      return std::make_unique<CustomTranslator>(m_specialValues);
    }

    void Visit(const MemberAccessExpression& expression) override {
      expression.GetExpression()->Apply(*this);
      auto valueExpression =
        StaticCast<std::unique_ptr<EvaluatorNode<TestEntry>>>(GetEvaluator());
      if(expression.GetName() == "is_special") {
        SetEvaluator(MakeFunctionEvaluatorNode(
          [specialValues = m_specialValues] (const TestEntry& entry) {
            return std::find_if(specialValues.begin(), specialValues.end(),
              [&] (auto specialValue) {
                return entry.m_value == specialValue;
              }) != specialValues.end();
        }, std::move(valueExpression)));
      } else {
        EvaluatorTranslator<CustomQueryTypes>::Visit(expression);
      }
    }
  };
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

  TEST_CASE("custom_translator") {
    auto specialValues = std::vector{3, 5, 9};
    auto dataStore = LocalDataStore<
      BasicQuery<std::string>, TestEntry, CustomTranslator>(specialValues);
    auto timeClient = IncrementalTimeClient();
    for(auto i = 1; i <= 10; ++i) {
      auto value = SequencedValue(
        IndexedValue(TestEntry{i, timeClient.GetTime()}, "hello"),
        Beam::Queries::Sequence(i));
      dataStore.Store(value);
    }
    auto specialQuery = BasicQuery<std::string>();
    specialQuery.SetIndex("hello");
    specialQuery.SetRange(Range::Historical());
    specialQuery.SetSnapshotLimit(SnapshotLimit::Unlimited());
    specialQuery.SetFilter(MemberAccessExpression("is_special",
      NativeDataType<bool>(),
      ParameterExpression(0, NativeDataType<TestEntry>())));
    auto specialEntries = dataStore.Load(specialQuery);
    REQUIRE(specialEntries.size() == 3);
    REQUIRE(specialEntries[0]->m_value == 3);
    REQUIRE(specialEntries[1]->m_value == 5);
    REQUIRE(specialEntries[2]->m_value == 9);
  }
}
