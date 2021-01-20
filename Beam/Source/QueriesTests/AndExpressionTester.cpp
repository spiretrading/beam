#include <doctest/doctest.h>
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("AndExpression") {
  TEST_CASE("constructor") {
    {
      auto expression = AndExpression(ConstantExpression(true),
        ConstantExpression(true));
      REQUIRE(expression.GetType() == BoolType());
      auto leftValue = expression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = expression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(true));
    }
    {
      auto expression = AndExpression(ConstantExpression(true),
        ConstantExpression(false));
      REQUIRE(expression.GetType() == BoolType());
      auto leftValue = expression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = expression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(false));
    }
    {
      auto expression = AndExpression(ConstantExpression(false),
        ConstantExpression(true));
      REQUIRE(expression.GetType() == BoolType());
      auto leftValue = expression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = expression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(true));
    }
    {
      auto expression = AndExpression(ConstantExpression(false),
        ConstantExpression(false));
      REQUIRE(expression.GetType() == BoolType());
      auto leftValue = expression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = expression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(false));
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

  TEST_CASE("empty_make_and_expression") {
    auto subexpressions = std::vector<Expression>();
    auto expression = MakeAndExpression(subexpressions.begin(),
      subexpressions.end());
    REQUIRE_NOTHROW(expression.DynamicCast<ConstantExpression>());
    REQUIRE_NOTHROW(expression.StaticCast<
      ConstantExpression>().GetType().DynamicCast<BoolType>());
    REQUIRE(expression.StaticCast<
      ConstantExpression>().GetValue()->GetValue<bool>() == false);
  }

  TEST_CASE("single_make_and_expression") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      auto expression = MakeAndExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<ConstantExpression>());
      REQUIRE_NOTHROW(expression.StaticCast<
        ConstantExpression>().GetType().DynamicCast<BoolType>());
      REQUIRE(expression.StaticCast<
        ConstantExpression>().GetValue()->GetValue<bool>() == true);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      auto expression = MakeAndExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<ConstantExpression>());
      REQUIRE_NOTHROW(expression.StaticCast<
        ConstantExpression>().GetType().DynamicCast<BoolType>());
      REQUIRE(expression.StaticCast<
        ConstantExpression>().GetValue()->GetValue<bool>() == false);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(0));
      REQUIRE_THROWS_AS(
        MakeAndExpression(subexpressions.begin(), subexpressions.end()),
        TypeCompatibilityException);
    }
  }

  TEST_CASE("two_sub_expression_make_and_expression") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(false));
      auto expression = MakeAndExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<AndExpression>());
      auto andExpression = expression.StaticCast<AndExpression>();
      REQUIRE(andExpression.GetType() == BoolType());
      auto leftValue = andExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = andExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(false));
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(true));
      auto expression = MakeAndExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<AndExpression>());
      auto andExpression = expression.StaticCast<AndExpression>();
      REQUIRE(andExpression.GetType() == BoolType());
      auto leftValue = andExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = andExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(true));
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(false));
      auto expression = MakeAndExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<AndExpression>());
      auto andExpression = expression.StaticCast<AndExpression>();
      REQUIRE(andExpression.GetType() == BoolType());
      auto leftValue = andExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = andExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(false));
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(true));
      auto expression = MakeAndExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<AndExpression>());
      auto andExpression = expression.StaticCast<AndExpression>();
      REQUIRE(andExpression.GetType() == BoolType());
      auto leftValue = andExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = andExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(true));
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
        auto expression = MakeAndExpression(subexpressions.begin(),
          subexpressions.end());
        REQUIRE_NOTHROW(expression.DynamicCast<AndExpression>());
        auto andExpression = expression.StaticCast<AndExpression>();
        REQUIRE(andExpression.GetType() == BoolType());
        auto traversal = static_cast<const VirtualExpression*>(&andExpression);
        for(auto k = std::size_t(0); k < i - 1; ++k) {
          auto value = ((j & (1UL << k)) != 0);
          auto expression = static_cast<const AndExpression*>(traversal);
          REQUIRE_NOTHROW(
            expression->GetLeftExpression().DynamicCast<ConstantExpression>());
          auto leftValue = expression->GetLeftExpression().StaticCast<
            ConstantExpression>().GetValue().StaticCast<BoolValue>();
          REQUIRE(leftValue.GetValue<bool>() == value);
          traversal = &expression->GetRightExpression().StaticCast<
            VirtualExpression>();
        }
        auto value = ((j & (1UL << (i - 1))) != 0);
        auto finalExpression = static_cast<const ConstantExpression*>(
          traversal);
        auto finalValue = finalExpression->GetValue().StaticCast<BoolValue>();
        REQUIRE(finalValue.GetValue<bool>() == value);
      }
    }
  }
}
