#ifndef BEAM_QUERIES_TESTS_TEST_ENTRY_HPP
#define BEAM_QUERIES_TESTS_TEST_ENTRY_HPP
#include <ostream>
#include <string>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Serialization/ShuttleDateTime.hpp"

namespace Beam::Tests {

  /** Represents a data store entry used for testing purposes. */
  struct TestEntry {

    /** The test value to store. */
    int m_value;

    /** The test timestamp. */
    boost::posix_time::ptime m_timestamp;

    bool operator ==(const TestEntry&) const = default;
  };

  using SequencedTestEntry = SequencedValue<TestEntry>;
  using SequencedIndexedTestEntry =
    SequencedValue<IndexedValue<TestEntry, std::string>>;

  /** Defines the queryable types for testing. */
  struct TestQueryTypes {
    using NativeTypes =
      boost::mp11::mp_push_back<QueryTypes::NativeTypes, TestEntry>;
    using ValueTypes =
      boost::mp11::mp_push_back<QueryTypes::ValueTypes, TestEntry>;
    using ComparableTypes = QueryTypes::ComparableTypes;
  };

  std::ostream& operator <<(std::ostream& out, const TestEntry& entry);

  /** Translates test Expressions into EvaluatorNodes for testing purposes. */
  class TestTranslator : public EvaluatorTranslator<TestQueryTypes> {
    public:
      std::unique_ptr<EvaluatorTranslator<TestQueryTypes>>
        make_translator() const override;

    protected:
      void visit(const MemberAccessExpression& expression) override;
  };

  template<typename DataStore>
  SequencedIndexedTestEntry store(DataStore& data_store, std::string index,
      int value, boost::posix_time::ptime timestamp, Beam::Sequence sequence) {
    auto entry = SequencedValue(
      IndexedValue(TestEntry(value, timestamp), std::move(index)), sequence);
    data_store.store(entry);
    return entry;
  }

  template<typename DataStore>
  void test_query(DataStore& data_store, std::string index,
      const Beam::Range& range, const SnapshotLimit& limit,
      const std::vector<SequencedTestEntry>& expected) {
    auto query = BasicQuery<std::string>();
    query.set_index(std::move(index));
    query.set_range(range);
    query.set_snapshot_limit(limit);
    auto result = data_store.load(query);
    REQUIRE(expected == result);
  }
}

namespace Beam {
  template<>
  struct Shuttle<Tests::TestEntry> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Tests::TestEntry& value, unsigned int version) const {
      shuttle.shuttle("value", value.m_value);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
