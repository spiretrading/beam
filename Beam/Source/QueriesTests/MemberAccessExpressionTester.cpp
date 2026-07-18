module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>
#include "Beam/Queries/MemberAccessExpression.hpp"

module Beam;

using namespace Beam;

TEST_SUITE("MemberAccessExpression") {
  TEST_CASE("constructor") {
    auto expression =
      MemberAccessExpression("x", typeid(int), ConstantExpression(1));
    REQUIRE(expression.get_name() == "x");
    REQUIRE(expression.get_type() == typeid(int));
  }

  TEST_CASE("stream") {
    auto expression =
      MemberAccessExpression("x", typeid(int), ConstantExpression(1));
    REQUIRE(to_string(expression) == "1.x");
  }
}
