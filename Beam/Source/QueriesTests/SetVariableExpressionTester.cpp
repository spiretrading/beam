#include <doctest/doctest.h>
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("SetVariableExpression") {
  TEST_CASE("accessors") {
    auto expression = SetVariableExpression("var", ConstantExpression(123));
    REQUIRE(expression.get_name() == "var");
    REQUIRE(expression.get_value().get_type() == typeid(int));
    REQUIRE(expression.get_type() == typeid(int));
  }

  TEST_CASE("stream") {
    auto expression = SetVariableExpression("count", ConstantExpression(123));
    REQUIRE(to_string(expression) == "(set count 123)");
  }
}
