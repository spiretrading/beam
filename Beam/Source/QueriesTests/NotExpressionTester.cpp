#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"

using namespace Beam;

TEST_SUITE("NotExpression") {
  TEST_CASE("constructor") {
    {
      auto not_expression = NotExpression(ConstantExpression(true));
      REQUIRE(not_expression.get_type() == typeid(bool));
      auto operand = not_expression.get_operand().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(operand);
    }
    {
      auto not_expression = NotExpression(ConstantExpression(false));
      REQUIRE(not_expression.get_type() == typeid(bool));
      auto operand = not_expression.get_operand().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(!operand);
    }
  }

  TEST_CASE("invalid_constructor") {
    REQUIRE_THROWS_AS(
      NotExpression(ConstantExpression(0)), TypeCompatibilityException);
  }
}
