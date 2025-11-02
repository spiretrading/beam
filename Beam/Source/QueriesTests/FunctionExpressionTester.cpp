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

  TEST_CASE("mixed_operators") {
    require_standard_function(
      ConstantExpression(1) + 2, ADDITION_NAME, typeid(int), std::vector{1, 2});
    require_standard_function(
      3 + ConstantExpression(4), ADDITION_NAME, typeid(int), std::vector{3, 4});
    require_standard_function(ConstantExpression(5.0) - 6.0,
      SUBTRACTION_NAME, typeid(double), std::vector{5.0, 6.0});
    require_standard_function(7.0 - ConstantExpression(8.0),
      SUBTRACTION_NAME, typeid(double), std::vector{7.0, 8.0});
    require_standard_function(ConstantExpression(9) * 10,
      MULTIPLICATION_NAME, typeid(int), std::vector{9, 10});
    require_standard_function(11 * ConstantExpression(12),
      MULTIPLICATION_NAME, typeid(int), std::vector{11, 12});
    require_standard_function(ConstantExpression(13) / 14,
      DIVISION_NAME, typeid(int), std::vector{13, 14});
    require_standard_function(15 / ConstantExpression(16),
      DIVISION_NAME, typeid(int), std::vector{15, 16});
    require_standard_function(ConstantExpression(17) < 18,
      LESS_NAME, typeid(bool), std::vector{17, 18});
    require_standard_function(19 < ConstantExpression(20),
      LESS_NAME, typeid(bool), std::vector{19, 20});
    require_standard_function(ConstantExpression(21) <= 22,
      LESS_EQUALS_NAME, typeid(bool), std::vector{21, 22});
    require_standard_function(23 <= ConstantExpression(24),
      LESS_EQUALS_NAME, typeid(bool), std::vector{23, 24});
    require_standard_function(ConstantExpression(25) == 26,
      EQUALS_NAME, typeid(bool), std::vector{25, 26});
    require_standard_function(27 == ConstantExpression(28),
      EQUALS_NAME, typeid(bool), std::vector{27, 28});
    require_standard_function(ConstantExpression(29) != 30,
      NOT_EQUALS_NAME, typeid(bool), std::vector{29, 30});
    require_standard_function(31 != ConstantExpression(32),
      NOT_EQUALS_NAME, typeid(bool), std::vector{31, 32});
    require_standard_function(ConstantExpression(33) >= 34,
      GREATER_EQUALS_NAME, typeid(bool), std::vector{33, 34});
    require_standard_function(35 >= ConstantExpression(36),
      GREATER_EQUALS_NAME, typeid(bool), std::vector{35, 36});
    require_standard_function(ConstantExpression(37) > 38,
      GREATER_NAME, typeid(bool), std::vector{37, 38});
    require_standard_function(39 > ConstantExpression(40),
      GREATER_NAME, typeid(bool), std::vector{39, 40});
    require_standard_function(max(ConstantExpression(41), 42),
      MAX_NAME, typeid(int), std::vector{41, 42});
    require_standard_function(max(43, ConstantExpression(44)),
      MAX_NAME, typeid(int), std::vector{43, 44});
    require_standard_function(min(ConstantExpression(45), 46),
      MIN_NAME, typeid(int), std::vector{45, 46});
    require_standard_function(min(47, ConstantExpression(48)),
      MIN_NAME, typeid(int), std::vector{47, 48});
  }
}
