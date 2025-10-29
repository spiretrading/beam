#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/MemberAccessExpression.hpp"

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
    auto ss = std::stringstream();
    ss << expression;
    REQUIRE(ss.str() == "1.x");
  }
}
