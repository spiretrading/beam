#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"

using namespace Beam;
using namespace Beam::Queries;

namespace {
  template<typename T>
  void require_standard_function(const FunctionExpression& expression,
      const std::string& name, const DataType& type,
      const std::vector<T>& parameters) {
    REQUIRE(expression.GetName() == name);
    REQUIRE(expression.GetType() == type);
    REQUIRE(expression.GetParameters().size() == parameters.size());
    for(auto i = 0; i != expression.GetParameters().size(); ++i) {
      auto parameter = expression.GetParameters()[i];
      REQUIRE(typeid(*parameter) == typeid(ConstantExpression));
      auto value = parameter.StaticCast<ConstantExpression>();
      REQUIRE(value.GetValue()->GetValue<T>() == parameters[i]);
    }
  }
}

TEST_SUITE("FunctionExpression") {
  TEST_CASE("empty_function") {
    auto parameters = std::vector<Expression>();
    auto function = FunctionExpression("empty", DecimalType(), parameters);
    REQUIRE(function.GetName() == "empty");
    REQUIRE(function.GetType() == DecimalType());
    REQUIRE(function.GetParameters().empty());
  }

  TEST_CASE("unary_function") {
    auto parameters = std::vector<Expression>();
    parameters.push_back(ConstantExpression(std::string("hello world")));
    auto function = FunctionExpression("unary", BoolType(), parameters);
    REQUIRE(function.GetName() == "unary");
    REQUIRE(function.GetType() == BoolType());
    REQUIRE(function.GetParameters().size() == 1);
    auto& p1 = *function.GetParameters()[0];
    REQUIRE(typeid(p1) == typeid(ConstantExpression));
    auto c1 = function.GetParameters()[0].StaticCast<ConstantExpression>();
    REQUIRE(c1.GetValue()->GetValue<std::string>() == "hello world");
  }

  TEST_CASE("binary_function") {
    auto parameters = std::vector<Expression>();
    parameters.push_back(ConstantExpression(5));
    parameters.push_back(ConstantExpression(6));
    auto function = FunctionExpression("binary", IntType(), parameters);
    REQUIRE(function.GetName() == "binary");
    REQUIRE(function.GetType() == IntType());
    REQUIRE(function.GetParameters().size() == 2);
    auto& p1 = *function.GetParameters()[0];
    REQUIRE(typeid(p1) == typeid(ConstantExpression));
    auto c1 = function.GetParameters()[0].StaticCast<ConstantExpression>();
    REQUIRE(c1.GetValue()->GetValue<int>() == 5);
    auto& p2 = *function.GetParameters()[1];
    REQUIRE(typeid(p2) == typeid(ConstantExpression));
    auto c2 = function.GetParameters()[1].StaticCast<ConstantExpression>();
    REQUIRE(c2.GetValue()->GetValue<int>() == 6);
  }

  TEST_CASE("standard_functions") {
    require_standard_function(ConstantExpression(1) + ConstantExpression(2),
      ADDITION_NAME, IntType(), std::vector{1, 2});
    require_standard_function(ConstantExpression(2.0) - ConstantExpression(3.0),
      SUBTRACTION_NAME, DecimalType(), std::vector{2.0, 3.0});
    require_standard_function(ConstantExpression(4) * ConstantExpression(5),
      MULTIPLICATION_NAME, IntType(), std::vector{4, 5});
    require_standard_function(ConstantExpression(5) / ConstantExpression(6),
      DIVISION_NAME, IntType(), std::vector{5, 6});
    require_standard_function(ConstantExpression(10) < ConstantExpression(20),
      LESS_NAME, BoolType(), std::vector{10, 20});
    require_standard_function(ConstantExpression(30) <= ConstantExpression(40),
      LESS_EQUALS_NAME, BoolType(), std::vector{30, 40});
    require_standard_function(ConstantExpression(5) == ConstantExpression(6),
      EQUALS_NAME, BoolType(), std::vector{5, 6});
    require_standard_function(ConstantExpression(5) != ConstantExpression(6),
      NOT_EQUALS_NAME, BoolType(), std::vector{5, 6});
    require_standard_function(ConstantExpression(10) >= ConstantExpression(20),
      GREATER_EQUALS_NAME, BoolType(), std::vector{10, 20});
    require_standard_function(ConstantExpression(30) > ConstantExpression(40),
      GREATER_NAME, BoolType(), std::vector{30, 40});
  }
}
