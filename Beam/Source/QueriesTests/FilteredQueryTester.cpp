#include <doctest/doctest.h>
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("FilteredQuery") {
  TEST_CASE("constructor") {
    auto query = FilteredQuery();
    REQUIRE(query.get_filter().get_type() == typeid(bool));
  }

  TEST_CASE("construct_boolean_filter") {
    auto expression = Expression(ConstantExpression(true));
    auto query = FilteredQuery(expression);
    REQUIRE(query.get_filter().get_type() == typeid(bool));
  }

  TEST_CASE("construct_non_boolean_filter") {
    REQUIRE_THROWS_AS(
      FilteredQuery(ConstantExpression(1)), TypeCompatibilityException);
  }

  TEST_CASE("set_filter") {
    auto query = FilteredQuery();
    query.set_filter(ConstantExpression(false));
    REQUIRE(query.get_filter().get_type() == typeid(bool));
    REQUIRE_THROWS_AS(query.set_filter(Expression(ConstantExpression(1))),
      TypeCompatibilityException);
  }

  TEST_CASE("stream") {
    auto query = FilteredQuery();
    REQUIRE(to_string(query) == "true");
  }

  TEST_CASE("test_filter") {
    auto true_evaluator = translate(ConstantExpression(true));
    auto false_evaluator = translate(ConstantExpression(false));
    REQUIRE(test_filter(*true_evaluator, 123));
    REQUIRE(!test_filter(*false_evaluator, 123));
  }
}
