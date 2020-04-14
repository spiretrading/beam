#include <doctest/doctest.h>
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("ParameterExpression") {
  TEST_CASE("constructor") {
    auto parameterA = ParameterExpression(0, BoolType());
    REQUIRE(parameterA.GetIndex() == 0);
    REQUIRE(parameterA.GetType() == BoolType());
    auto parameterB = ParameterExpression(1, DecimalType());
    REQUIRE(parameterB.GetIndex() == 1);
    REQUIRE(parameterB.GetType() == DecimalType());
    auto parameterC = ParameterExpression(2, StringType());
    REQUIRE(parameterC.GetIndex() == 2);
    REQUIRE(parameterC.GetType() == StringType());
  }
}
