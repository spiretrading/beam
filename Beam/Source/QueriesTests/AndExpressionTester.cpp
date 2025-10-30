#include <doctest/doctest.h>
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("AndExpression") {
  TEST_CASE("constructor") {
    {
      auto expression =
        AndExpression(ConstantExpression(true), ConstantExpression(true));
      REQUIRE(expression.get_type() == typeid(bool));
      auto left_value =
        expression.get_left().as<ConstantExpression>().get_value().as<bool>();
      auto right_value =
        expression.get_right().as<ConstantExpression>().get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(right_value);
    }
    {
      auto expression =
        AndExpression(ConstantExpression(true), ConstantExpression(false));
      REQUIRE(expression.get_type() == typeid(bool));
      auto left_value =
        expression.get_left().as<ConstantExpression>().get_value().as<bool>();
      auto right_value =
        expression.get_right().as<ConstantExpression>().get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(!right_value);
    }
    {
      auto expression =
        AndExpression(ConstantExpression(false), ConstantExpression(true));
      REQUIRE(expression.get_type() == typeid(bool));
      auto left_value =
        expression.get_left().as<ConstantExpression>().get_value().as<bool>();
      auto right_value =
        expression.get_right().as<ConstantExpression>().get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(right_value);
    }
    {
      auto expression =
        AndExpression(ConstantExpression(false), ConstantExpression(false));
      REQUIRE(expression.get_type() == typeid(bool));
      auto left_value =
        expression.get_left().as<ConstantExpression>().get_value().as<bool>();
      auto right_value =
        expression.get_right().as<ConstantExpression>().get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(!right_value);
    }
  }

  TEST_CASE("invalid_constructor") {
    REQUIRE_THROWS_AS(AndExpression(ConstantExpression(0),
      ConstantExpression(true)), TypeCompatibilityException);
    REQUIRE_THROWS_AS(AndExpression(ConstantExpression(true),
      ConstantExpression(0)), TypeCompatibilityException);
    REQUIRE_THROWS_AS(AndExpression(ConstantExpression(0),
      ConstantExpression(0)), TypeCompatibilityException);
  }

  TEST_CASE("empty_conjunction") {
    auto subexpressions = std::vector<Expression>();
    auto expression = conjunction(subexpressions.begin(), subexpressions.end());
    REQUIRE(!expression.as<ConstantExpression>().get_value().as<bool>());
  }

  TEST_CASE("single_conjunction") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      auto expression =
        conjunction(subexpressions.begin(), subexpressions.end());
      REQUIRE(expression.as<ConstantExpression>().get_value().as<bool>());
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      auto expression =
        conjunction(subexpressions.begin(), subexpressions.end());
      REQUIRE(!expression.as<ConstantExpression>().get_value().as<bool>());
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(0));
      REQUIRE_THROWS_AS(
        conjunction(subexpressions.begin(), subexpressions.end()),
        TypeCompatibilityException);
    }
  }

  TEST_CASE("two_sub_expression_make_and_expression") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(false));
      auto expression =
        conjunction(subexpressions.begin(), subexpressions.end());
      auto and_expression = expression.as<AndExpression>();
      REQUIRE(and_expression.get_type() == typeid(bool));
      auto left_value = and_expression.get_left().as<
        ConstantExpression>().get_value().as<bool>();
      auto right_value = and_expression.get_right().as<
        ConstantExpression>().get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(!right_value);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(true));
      auto expression =
        conjunction(subexpressions.begin(), subexpressions.end());
      auto and_expression = expression.as<AndExpression>();
      REQUIRE(and_expression.get_type() == typeid(bool));
      auto left_value = and_expression.get_left().as<
        ConstantExpression>().get_value().as<bool>();
      auto right_value = and_expression.get_right().as<
        ConstantExpression>().get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(right_value);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(false));
      auto expression =
        conjunction(subexpressions.begin(), subexpressions.end());
      auto and_expression = expression.as<AndExpression>();
      REQUIRE(and_expression.get_type() == typeid(bool));
      auto left_value = and_expression.get_left().as<
        ConstantExpression>().get_value().as<bool>();
      auto right_value = and_expression.get_right().as<
        ConstantExpression>().get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(!right_value);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(true));
      auto expression =
        conjunction(subexpressions.begin(), subexpressions.end());
      auto and_expression = expression.as<AndExpression>();
      REQUIRE(and_expression.get_type() == typeid(bool));
      auto left_value = and_expression.get_left().as<
        ConstantExpression>().get_value().as<bool>();
      auto right_value = and_expression.get_right().as<
        ConstantExpression>().get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(right_value);
    }
  }

  TEST_CASE("multiple_sub_expression_make_and_expression") {
    const auto LOWER_BOUND = std::size_t(3);
    const auto UPPER_BOUND = std::size_t(5);
    for(auto i = LOWER_BOUND; i < UPPER_BOUND; ++i) {
      for(auto j = std::size_t(0); j < (1UL << i); ++j) {
        auto subexpressions = std::vector<Expression>();
        for(auto k = std::size_t(0); k < i; ++k) {
          auto value = ((j & (1UL << k)) != 0);
          subexpressions.push_back(ConstantExpression(value));
        }
        auto expression =
          conjunction(subexpressions.begin(), subexpressions.end());
        auto and_expression = expression.as<AndExpression>();
        REQUIRE(and_expression.get_type() == typeid(bool));
        auto traversal = static_cast<const VirtualExpression*>(&and_expression);
        for(auto k = std::size_t(0); k < i - 1; ++k) {
          auto value = ((j & (1UL << k)) != 0);
          auto expression = static_cast<const AndExpression*>(traversal);
          auto left_value = expression->get_left().as<
            ConstantExpression>().get_value().as<bool>();
          REQUIRE(left_value == value);
          traversal = &expression->get_right().as<VirtualExpression>();
        }
        auto value = ((j & (1UL << (i - 1))) != 0);
        auto final_expression =
          static_cast<const ConstantExpression*>(traversal);
        auto final_value = final_expression->get_value().as<bool>();
        REQUIRE(final_value == value);
      }
    }
  }

  TEST_CASE("stream") {
    {
      auto expression =
        AndExpression(ConstantExpression(true), ConstantExpression(false));
      REQUIRE(to_string(expression) == "(and true false)");
    }
    {
      auto expression = AndExpression(ConstantExpression(true),
        AndExpression(ConstantExpression(false), ConstantExpression(true)));
      REQUIRE(to_string(expression) == "(and true (and false true))");
    }
  }
}
