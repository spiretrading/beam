#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ExpressionQuery.hpp"
#include "Beam/QueriesTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ExpressionQuery") {
  TEST_CASE("constructor") {
    auto query = ExpressionQuery();
    REQUIRE(query.get_update_policy() == ExpressionQuery::UpdatePolicy::ALL);
    REQUIRE(query.get_expression().get_type() == typeid(bool));
  }

  TEST_CASE("set_update_policy") {
    auto query = ExpressionQuery();
    query.set_update_policy(ExpressionQuery::UpdatePolicy::CHANGE);
    REQUIRE(query.get_update_policy() == ExpressionQuery::UpdatePolicy::CHANGE);
  }

  TEST_CASE("construct_with_expression") {
    auto expression = ConstantExpression(false);
    auto query = ExpressionQuery(expression);
    REQUIRE(query.get_expression().get_type() == typeid(bool));
  }

  TEST_CASE("set_expression") {
    auto query = ExpressionQuery();
    auto expression = ConstantExpression(false);
    query.set_expression(expression);
    REQUIRE(query.get_expression().get_type() == typeid(bool));
  }

  TEST_CASE("stream_update_policy") {
    REQUIRE(to_string(ExpressionQuery::UpdatePolicy::ALL) == "ALL");
    REQUIRE(to_string(ExpressionQuery::UpdatePolicy::CHANGE) == "CHANGE");
  }

  TEST_CASE("stream") {
    auto query = ExpressionQuery(ConstantExpression(123));
    query.set_update_policy(ExpressionQuery::UpdatePolicy::CHANGE);
    REQUIRE(to_string(query) == "(CHANGE 123)");
    test_query_round_trip_shuttle(query, [&] (auto&& received) {
      REQUIRE(received.get_update_policy() == query.get_update_policy());
      REQUIRE((received.get_expression().
        template as<ConstantExpression>().get_value() == 123));
    });
  }
}
