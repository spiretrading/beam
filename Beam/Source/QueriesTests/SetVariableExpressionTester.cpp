#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Queries/SetVariableExpression.hpp"

using namespace Beam;

TEST_SUITE("SetVariableExpression") {
  TEST_CASE("accessors") {
    auto expression = SetVariableExpression("var", ConstantExpression(123));
    REQUIRE(expression.get_name() == "var");
    REQUIRE(expression.get_value().get_type() == typeid(int));
    REQUIRE(expression.get_type() == typeid(int));
  }

  TEST_CASE("stream") {
    auto buffer = std::stringstream();
    auto expression = SetVariableExpression("count", ConstantExpression(123));
    buffer << expression;
    REQUIRE(buffer.str() == "(set count 123)");
  }
}
