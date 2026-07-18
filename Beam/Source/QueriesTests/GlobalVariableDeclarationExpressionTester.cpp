module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>

module Beam;

using namespace Beam;

TEST_SUITE("GlobalVariableDeclarationExpression") {
  TEST_CASE("construct_from_ints") {
    auto expression = GlobalVariableDeclarationExpression(
      "x", ConstantExpression(1), ConstantExpression(3));
    REQUIRE(expression.get_name() == "x");
    REQUIRE(expression.get_type() == typeid(int));
  }

  TEST_CASE("construct_from_string_body") {
    auto expression = GlobalVariableDeclarationExpression(
      "message", ConstantExpression("hello"), ConstantExpression("world"));
    REQUIRE(expression.get_name() == "message");
    REQUIRE(expression.get_type() == typeid(std::string));
  }

  TEST_CASE("stream") {
    auto expression = GlobalVariableDeclarationExpression(
      "x", ConstantExpression(1), ConstantExpression(2));
    REQUIRE(to_string(expression) == "(global (x 1) 2)");
  }
}
