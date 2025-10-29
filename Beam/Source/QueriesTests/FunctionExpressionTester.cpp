#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"

using namespace Beam;

namespace {
  template<typename T>
  void require_standard_function(const FunctionExpression& expression,
      const std::string& name, std::type_index type,
      const std::vector<T>& parameters) {
    REQUIRE(expression.get_name() == name);
    REQUIRE(expression.get_type() == type);
    REQUIRE(expression.get_parameters().size() == parameters.size());
    for(auto i = 0; i != expression.get_parameters().size(); ++i) {
      auto parameter = expression.get_parameters()[i];
      auto& value = parameter.as<ConstantExpression>();
      REQUIRE(value.get_value().as<T>() == parameters[i]);
    }
  }
}

TEST_SUITE("FunctionExpression") {
  TEST_CASE("empty_function") {
    auto parameters = std::vector<Expression>();
    auto function = FunctionExpression("empty", typeid(double), parameters);
    REQUIRE(function.get_name() == "empty");
    REQUIRE(function.get_type() == typeid(double));
    REQUIRE(function.get_parameters().empty());
  }

  TEST_CASE("unary_function") {
    auto parameters = std::vector<Expression>();
    parameters.push_back(ConstantExpression(std::string("hello world")));
    auto function = FunctionExpression("unary", typeid(bool), parameters);
    REQUIRE(function.get_name() == "unary");
    REQUIRE(function.get_type() == typeid(bool));
    REQUIRE(function.get_parameters().size() == 1);
    auto& c1 = function.get_parameters()[0].as<ConstantExpression>();
    REQUIRE(c1.get_value().as<std::string>() == "hello world");
  }

  TEST_CASE("binary_function") {
    auto parameters = std::vector<Expression>();
    parameters.push_back(ConstantExpression(5));
    parameters.push_back(ConstantExpression(6));
    auto function = FunctionExpression("binary", typeid(int), parameters);
    REQUIRE(function.get_name() == "binary");
    REQUIRE(function.get_type() == typeid(int));
    REQUIRE(function.get_parameters().size() == 2);
    auto& c1 = function.get_parameters()[0].as<ConstantExpression>();
    REQUIRE(c1.get_value().as<int>() == 5);
    auto& c2 = function.get_parameters()[1].as<ConstantExpression>();
    REQUIRE(c2.get_value().as<int>() == 6);
  }

  TEST_CASE("standard_functions") {
    require_standard_function(ConstantExpression(1) + ConstantExpression(2),
      ADDITION_NAME, typeid(int), std::vector{1, 2});
    require_standard_function(ConstantExpression(2.0) - ConstantExpression(3.0),
      SUBTRACTION_NAME, typeid(double), std::vector{2.0, 3.0});
    require_standard_function(ConstantExpression(4) * ConstantExpression(5),
      MULTIPLICATION_NAME, typeid(int), std::vector{4, 5});
    require_standard_function(ConstantExpression(5) / ConstantExpression(6),
      DIVISION_NAME, typeid(int), std::vector{5, 6});
    require_standard_function(ConstantExpression(10) < ConstantExpression(20),
      LESS_NAME, typeid(bool), std::vector{10, 20});
    require_standard_function(ConstantExpression(30) <= ConstantExpression(40),
      LESS_EQUALS_NAME, typeid(bool), std::vector{30, 40});
    require_standard_function(ConstantExpression(5) == ConstantExpression(6),
      EQUALS_NAME, typeid(bool), std::vector{5, 6});
    require_standard_function(ConstantExpression(5) != ConstantExpression(6),
      NOT_EQUALS_NAME, typeid(bool), std::vector{5, 6});
    require_standard_function(ConstantExpression(10) >= ConstantExpression(20),
      GREATER_EQUALS_NAME, typeid(bool), std::vector{10, 20});
    require_standard_function(ConstantExpression(30) > ConstantExpression(40),
      GREATER_NAME, typeid(bool), std::vector{30, 40});
  }
}
