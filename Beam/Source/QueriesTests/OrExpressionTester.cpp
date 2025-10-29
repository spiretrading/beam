#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"

using namespace Beam;

TEST_SUITE("OrExpression") {
  TEST_CASE("constructor") {
    {
      auto or_expression =
        OrExpression(ConstantExpression(true), ConstantExpression(true));
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(right_value);
    }
    {
      auto or_expression =
        OrExpression(ConstantExpression(true), ConstantExpression(false));
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(!right_value);
    }
    {
      auto or_expression =
        OrExpression(ConstantExpression(false), ConstantExpression(true));
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(right_value);
    }
    {
      auto or_expression =
        OrExpression(ConstantExpression(false), ConstantExpression(false));
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(!right_value);
    }
  }

  TEST_CASE("invalid_constructor") {
    REQUIRE_THROWS_AS(OrExpression(ConstantExpression(0),
      ConstantExpression(true)), TypeCompatibilityException);
    REQUIRE_THROWS_AS(OrExpression(ConstantExpression(true),
      ConstantExpression(0)), TypeCompatibilityException);
    REQUIRE_THROWS_AS(OrExpression(ConstantExpression(0),
      ConstantExpression(0)), TypeCompatibilityException);
  }

  TEST_CASE("empty_make_or_expression") {
    auto subexpressions = std::vector<Expression>();
    auto or_expression =
      disjunction(subexpressions.begin(), subexpressions.end());
    REQUIRE(!or_expression.as<ConstantExpression>().get_value().as<bool>());
  }

  TEST_CASE("single_make_or_expression") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      auto or_expression =
        disjunction(subexpressions.begin(), subexpressions.end());
      REQUIRE(or_expression.as<ConstantExpression>().get_value().as<bool>());
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      auto or_expression =
        disjunction(subexpressions.begin(), subexpressions.end());
      REQUIRE(!or_expression.as<ConstantExpression>().get_value().as<bool>());
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(0));
      REQUIRE_THROWS_AS(
        disjunction(subexpressions.begin(), subexpressions.end()),
        TypeCompatibilityException);
    }
  }

  TEST_CASE("two_sub_expression_make_or_expression") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(false));
      auto expression =
        disjunction(subexpressions.begin(), subexpressions.end());
      auto or_expression = expression.as<OrExpression>();
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(!right_value);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(true));
      auto expression =
        disjunction(subexpressions.begin(), subexpressions.end());
      auto or_expression = expression.as<OrExpression>();
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(!left_value);
      REQUIRE(right_value);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(false));
      auto expression =
        disjunction(subexpressions.begin(), subexpressions.end());
      auto or_expression = expression.as<OrExpression>();
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(!right_value);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(true));
      auto expression =
        disjunction(subexpressions.begin(), subexpressions.end());
      auto or_expression = expression.as<OrExpression>();
      REQUIRE(or_expression.get_type() == typeid(bool));
      auto left_value = or_expression.get_left().as<ConstantExpression>().
        get_value().as<bool>();
      auto right_value = or_expression.get_right().as<ConstantExpression>().
        get_value().as<bool>();
      REQUIRE(left_value);
      REQUIRE(right_value);
    }
  }

  TEST_CASE("multiple_sub_expression_make_or_expression") {
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
          disjunction(subexpressions.begin(), subexpressions.end());
        auto or_expression = expression.as<OrExpression>();
        REQUIRE(or_expression.get_type() == typeid(bool));
        auto traversal = static_cast<const VirtualExpression*>(&or_expression);
        for(auto k = std::size_t(0); k < i - 1; ++k) {
          auto value = ((j & (1UL << k)) != 0);
          auto expression = static_cast<const OrExpression*>(traversal);
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
        OrExpression(ConstantExpression(true), ConstantExpression(false));
      auto ss = std::stringstream();
      ss << expression;
      REQUIRE(ss.str() == "(or true false)");
    }
    {
      auto expression = OrExpression(
        OrExpression(ConstantExpression(true), ConstantExpression(false)),
        ConstantExpression(true));
      auto ss = std::stringstream();
      ss << expression;
      REQUIRE(ss.str() == "(or (or true false) true)");
    }
  }
}
