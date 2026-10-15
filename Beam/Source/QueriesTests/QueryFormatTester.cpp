#include <doctest/doctest.h>
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/ExpressionQuery.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/PagedQuery.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace boost::posix_time;

namespace {
  struct StreamValue {
    int m_value;

    bool operator ==(const StreamValue&) const = default;
  };

  std::ostream& operator <<(std::ostream& out, const StreamValue& value) {
    return out << "value:" << value.m_value;
  }

  struct FormattedValue {
    int m_value;

    bool operator ==(const FormattedValue&) const = default;
  };

  class StreamExpression : public VirtualExpression {
    public:
      std::type_index get_type() const override {
        return typeid(bool);
      }

      void apply(ExpressionVisitor& visitor) const override {
        visitor.visit(*this);
      }

    protected:
      std::ostream& to_stream(std::ostream& out) const override {
        return out << "streamed";
      }
  };
}

template<>
class std::formatter<FormattedValue> : public std::formatter<std::string_view> {
  public:
    auto format(const FormattedValue& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format("value:{}", value.m_value), context);
    }
};

namespace {
  void test_format(const auto& value, const std::string& expected) {
    REQUIRE(std::format("{}", value) == expected);
    REQUIRE(to_string(value) == expected);
    REQUIRE(std::format("{:*>{}}", value, expected.size() + 2) ==
      "**" + expected);
    REQUIRE(std::format("{:.3}", value) == expected.substr(0, 3));
  }
}

TEST_SUITE("QueryFormat") {
  TEST_CASE("format_dependencies") {
    test_format(Value(true), "true");
    test_format(Value('a'), "a");
    test_format(Value(42), "42");
    test_format(Value(1.23456789), "1.23457");
    test_format(Value(std::uint64_t(18446744073709551615ULL)),
      "18446744073709551615");
    test_format(Value("hello"), "\"hello\"");
    test_format(Value(time_from_string("2026-10-15 09:30:00.123456")),
      "2026-Oct-15 09:30:00.123456");
    test_format(Value(duration_from_string("-100:02:03.000001")),
      "-100:02:03.000001");
    test_format(Value(ptime(not_a_date_time)), "not-a-date-time");
    test_format(Value(ptime(pos_infin)), "+infinity");
    test_format(Value(ptime(neg_infin)), "-infinity");
    test_format(Value(time_duration(not_a_date_time)), "not-a-date-time");
    test_format(Value(time_duration(pos_infin)), "+infinity");
    test_format(Value(time_duration(neg_infin)), "-infinity");
    test_format(Value(seconds(0)), "00:00:00");
    test_format(NativeValue<int>(42), "42");
    test_format(ConstantExpression(true), "true");
    test_format(AndExpression(ConstantExpression(true),
      NotExpression(ConstantExpression(false))), "(and true (not false))");
    test_format(OrExpression(ConstantExpression(true),
      ConstantExpression(false)), "(or true false)");
    test_format(FunctionExpression("sum", typeid(int),
      {ConstantExpression(1), ConstantExpression(2)}), "(sum 1 2)");
    test_format(FunctionExpression("zero", typeid(int), {}), "(zero)");
    test_format(GlobalVariableDeclarationExpression("x",
      ConstantExpression(1), ConstantExpression(2)), "(global (x 1) 2)");
    test_format(MemberAccessExpression("x", typeid(int),
      ParameterExpression(0, typeid(int))), "(parameter 0).x");
    test_format(ReduceExpression(ConstantExpression(1),
      ConstantExpression(2), Value(3)), "(reduce 1 2 3)");
    test_format(SetVariableExpression("count", ConstantExpression(123)),
      "(set count 123)");
    test_format(VariableExpression("count", typeid(int)), "count");
    test_format(Expression(ParameterExpression(1, typeid(int))),
      "(parameter 1)");
  }

  TEST_CASE("format") {
    SUBCASE("stream_dependencies") {
      auto query = make_current_query(FormattedValue(7));
      query.set_filter(StreamExpression());
      REQUIRE(std::format("{}", query) ==
        "(value:7 Total (TAIL 1) IGNORE_CONTINUE streamed)");
      REQUIRE(std::format("{}", ExpressionQuery(StreamExpression())) ==
        "(ALL streamed)");
      REQUIRE(std::format("{}", FilteredQuery(StreamExpression())) ==
        "streamed");
      REQUIRE(std::format("{}", Value(StreamValue(7))) == "value:7");
      REQUIRE(std::format("{}", ConstantExpression(StreamValue(7))) ==
        "value:7");
      test_format(StreamExpression(), "streamed");
      test_format(NativeValue<StreamValue>(StreamValue(7)), "value:7");
    }
    SUBCASE("stream_only_payloads") {
      REQUIRE_FALSE((std::formattable<BasicQuery<StreamValue>, char>));
      REQUIRE_FALSE((std::formattable<IndexedQuery<StreamValue>, char>));
      REQUIRE_FALSE((std::formattable<PagedQuery<int, StreamValue>, char>));
      REQUIRE_FALSE((std::formattable<IndexedValue<int, StreamValue>, char>));
      REQUIRE_FALSE((std::formattable<SequencedValue<StreamValue>, char>));
      REQUIRE(to_string(make_current_query(StreamValue(7))) ==
        "(value:7 Total (TAIL 1) IGNORE_CONTINUE true)");
      REQUIRE(to_string(IndexedValue(10, StreamValue(7))) == "(value:7 10)");
    }
    SUBCASE("sequence") {
      REQUIRE(std::format("{}", Sequence(42)) == "42");
      REQUIRE(std::format("{:06}", Sequence(42)) == "000042");
      REQUIRE(std::format("{:x}", Sequence(42)) == "2a");
      REQUIRE(std::format("{}", Sequence::FIRST) == "0");
      REQUIRE(std::format("{}", Sequence::LAST) == "18446744073709551615");
      REQUIRE(std::format("{}", Sequence::PRESENT) == "18446744073709551614");
      REQUIRE(to_string(Sequence(42)) == "42");
    }
    SUBCASE("snapshot_limit") {
      test_format(SnapshotLimit::Type::HEAD, "HEAD");
      test_format(SnapshotLimit::Type::TAIL, "TAIL");
      test_format(static_cast<SnapshotLimit::Type>(99), "NONE");
      test_format(SnapshotLimit::NONE, "None");
      test_format(SnapshotLimit::UNLIMITED, "Unlimited");
      test_format(SnapshotLimit::from_head(10), "(HEAD 10)");
      test_format(SnapshotLimit::from_tail(5), "(TAIL 5)");
    }
    SUBCASE("range") {
      test_format(Range::EMPTY, "Empty");
      test_format(Range::TOTAL, "Total");
      test_format(Range::HISTORICAL, "(0 18446744073709551614)");
      test_format(Range::REAL_TIME,
        "(18446744073709551614 18446744073709551615)");
      test_format(Range(Sequence(1), Sequence(20)), "(1 20)");
      auto start = time_from_string("2026-10-15 09:30:00.123456");
      auto end = time_from_string("2026-10-15 16:00:00");
      test_format(Range(start, end),
        "(2026-Oct-15 09:30:00.123456 2026-Oct-15 16:00:00)");
      test_format(Range(start, Sequence(20)),
        "(2026-Oct-15 09:30:00.123456 20)");
    }
    SUBCASE("query_components") {
      test_format(IndexedQuery<int>(42), "42");
      test_format(IndexedQuery<bool>(true), "true");
      test_format(IndexedQuery<bool>(false), "false");
      test_format(IndexedQuery<FormattedValue>(FormattedValue(7)), "value:7");
      test_format(RangedQuery(Range::TOTAL), "Total");
      test_format(SnapshotLimitedQuery(SnapshotLimit::from_tail(5)),
        "(TAIL 5)");
      test_format(InterruptableQuery(InterruptionPolicy::IGNORE_CONTINUE),
        "IGNORE_CONTINUE");
      test_format(FilteredQuery(ConstantExpression(false)), "false");
    }
    SUBCASE("expression_query") {
      test_format(ExpressionQuery::UpdatePolicy::ALL, "ALL");
      test_format(ExpressionQuery::UpdatePolicy::CHANGE, "CHANGE");
      test_format(static_cast<ExpressionQuery::UpdatePolicy>(99), "NONE");
      test_format(ExpressionQuery(), "(ALL true)");
      auto query = ExpressionQuery(ConstantExpression(123));
      query.set_update_policy(ExpressionQuery::UpdatePolicy::CHANGE);
      test_format(query, "(CHANGE 123)");
    }
    SUBCASE("basic_query") {
      test_format(BasicQuery<int>(), "(0 Empty None BREAK_QUERY true)");
      test_format(BasicQuery<bool>(), "(false Empty None BREAK_QUERY true)");
      auto query = make_current_query(FormattedValue(7));
      query.set_filter(ConstantExpression(false));
      test_format(query, "(value:7 Total (TAIL 1) IGNORE_CONTINUE false)");
    }
    SUBCASE("paged_query") {
      auto numbers = PagedQuery<bool, double>();
      numbers.set_index(true);
      numbers.set_anchor(1.23456789);
      test_format(numbers, "(true None 1.23456789 true)");
      auto query = PagedQuery<std::string, FormattedValue>();
      query.set_index("cities");
      query.set_snapshot_limit(SnapshotLimit::from_head(20));
      test_format(query, "(cities (HEAD 20) true)");
      query.set_offset(3);
      test_format(query, "(cities (HEAD 20) 3 true)");
      query.set_anchor(FormattedValue(7));
      test_format(query, "(cities (HEAD 20) value:7 3 true)");
      query.set_offset(0);
      test_format(query, "(cities (HEAD 20) value:7 true)");
    }
    SUBCASE("indexed_value") {
      test_format(IndexedValue(1.23456789, std::string("index")),
        "(index 1.23456789)");
      test_format(IndexedValue(true, FormattedValue(7)), "(value:7 true)");
    }
    SUBCASE("sequenced_value") {
      test_format(SequencedValue(false, Sequence(42)), "(false 42)");
      test_format(SequencedValue(1.23456789, Sequence(42)), "(1.23456789 42)");
      test_format(SequencedValue(FormattedValue(7), Sequence(42)),
        "(value:7 42)");
      test_format(SequencedValue(
        IndexedValue(10, std::string("index")), Sequence(42)),
        "((index 10) 42)");
    }
  }
}
