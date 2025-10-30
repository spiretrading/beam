#include <doctest/doctest.h>
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("VariableExpression") {
  TEST_CASE("accessors") {
    auto expression = VariableExpression("var", typeid(int));
    REQUIRE(expression.get_name() == "var");
    REQUIRE(expression.get_type() == typeid(int));
  }

  TEST_CASE("stream") {
    auto expression = VariableExpression("count", typeid(bool));
    REQUIRE(to_string(expression) == "count");
  }
}
