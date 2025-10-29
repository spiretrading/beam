#include <doctest/doctest.h>
#include "Beam/Queries/ParameterExpression.hpp"

using namespace Beam;

TEST_SUITE("ParameterExpression") {
  TEST_CASE("constructor") {
    auto parameter_a = ParameterExpression(0, typeid(bool));
    REQUIRE(parameter_a.get_index() == 0);
    REQUIRE(parameter_a.get_type() == typeid(bool));
    auto parameter_b = ParameterExpression(1, typeid(double));
    REQUIRE(parameter_b.get_index() == 1);
    REQUIRE(parameter_b.get_type() == typeid(double));
    auto parameter_c = ParameterExpression(2, typeid(std::string));
    REQUIRE(parameter_c.get_index() == 2);
    REQUIRE(parameter_c.get_type() == typeid(std::string));
  }
}
