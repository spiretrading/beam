#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/NativeValue.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("ConstantExpression") {
  TEST_CASE("make_constant_expression") {
    auto intExpression = ConstantExpression(123);
    REQUIRE(intExpression.GetValue()->GetValue<int>() == 123);
    auto stringExpression = ConstantExpression(std::string("hello world"));
    REQUIRE(stringExpression.GetValue()->GetValue<std::string>() ==
      "hello world");
  }

  TEST_CASE("int") {
    auto constantExpression = ConstantExpression(NativeValue(123));
    REQUIRE(constantExpression.GetType() == NativeDataType<int>());
    REQUIRE(constantExpression.GetValue() == NativeValue(123));
    auto expression = Expression(constantExpression);
    REQUIRE(expression->GetType() == NativeDataType<int>());
  }

  TEST_CASE("decimal") {
    auto constantExpression = ConstantExpression(NativeValue(3.14));
    REQUIRE(constantExpression.GetType() == NativeDataType<double>());
    REQUIRE(constantExpression.GetValue() == NativeValue(3.14));
    auto expression = Expression(constantExpression);
    REQUIRE(expression->GetType() == NativeDataType<double>());
  }

  TEST_CASE("string") {
    auto constantExpression = ConstantExpression(NativeValue(
      std::string("hello world")));
    REQUIRE(constantExpression.GetType() == NativeDataType<std::string>());
    REQUIRE(constantExpression.GetValue() == NativeValue(
      std::string("hello world")));
    auto expression = Expression(constantExpression);
    REQUIRE(expression->GetType() == NativeDataType<std::string>());
  }
}
