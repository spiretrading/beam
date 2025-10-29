#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/ConstantExpression.hpp"

using namespace Beam;

TEST_SUITE("GlobalVariableDeclarationExpression") {
  TEST_CASE("construct_from_ints") {
    auto expression = GlobalVariableDeclarationExpression(
      "x", Expression(ConstantExpression(1)),
      Expression(ConstantExpression(3)));
    REQUIRE(expression.get_name() == "x");
    REQUIRE(expression.get_type() == typeid(int));
  }

  TEST_CASE("construct_from_string_body") {
    auto expression = GlobalVariableDeclarationExpression(
      "message", Expression(ConstantExpression("hello")),
      Expression(ConstantExpression("world")));
    REQUIRE(expression.get_name() == "message");
    REQUIRE(expression.get_type() == typeid(std::string));
  }

  TEST_CASE("stream") {
    auto expression = GlobalVariableDeclarationExpression(
      "x", Expression(ConstantExpression(1)),
      Expression(ConstantExpression(2)));
    auto stream = std::stringstream();
    stream << expression;
    REQUIRE(stream.str() == "(global (x 1) 2)");
  }
}
