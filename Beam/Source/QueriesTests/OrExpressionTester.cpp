#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("OrExpression") {
  TEST_CASE("constructor") {
    {
      auto orExpression = OrExpression(ConstantExpression(true),
        ConstantExpression(true));
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(true));
    }
    {
      auto orExpression = OrExpression(ConstantExpression(true),
        ConstantExpression(false));
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(false));
    }
    {
      auto orExpression = OrExpression(ConstantExpression(false),
        ConstantExpression(true));
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(true));
    }
    {
      auto orExpression = OrExpression(ConstantExpression(false),
        ConstantExpression(false));
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(false));
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
    auto orExpression = MakeOrExpression(subexpressions.begin(),
      subexpressions.end());
    REQUIRE_NOTHROW(orExpression.DynamicCast<ConstantExpression>());
    REQUIRE_NOTHROW(orExpression.StaticCast<
      ConstantExpression>().GetType().DynamicCast<BoolType>());
    REQUIRE(orExpression.StaticCast<
      ConstantExpression>().GetValue()->GetValue<bool>() == false);
  }

  TEST_CASE("single_make_or_expression") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      auto orExpression = MakeOrExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(orExpression.DynamicCast<ConstantExpression>());
      REQUIRE_NOTHROW(orExpression.StaticCast<
        ConstantExpression>().GetType().DynamicCast<BoolType>());
      REQUIRE(orExpression.StaticCast<
        ConstantExpression>().GetValue()->GetValue<bool>() == true);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      auto orExpression = MakeOrExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(orExpression.DynamicCast<ConstantExpression>());
      REQUIRE_NOTHROW(orExpression.StaticCast<
        ConstantExpression>().GetType().DynamicCast<BoolType>());
      REQUIRE(orExpression.StaticCast<
        ConstantExpression>().GetValue()->GetValue<bool>() == false);
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(0));
      REQUIRE_THROWS_AS(
        MakeOrExpression(subexpressions.begin(), subexpressions.end()),
        TypeCompatibilityException);
    }
  }

  TEST_CASE("two_sub_expression_make_or_expression") {
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(false));
      auto expression = MakeOrExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<OrExpression>());
      auto orExpression = expression.StaticCast<OrExpression>();
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(false));
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(false));
      subexpressions.push_back(ConstantExpression(true));
      auto expression = MakeOrExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<OrExpression>());
      auto orExpression = expression.StaticCast<OrExpression>();
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(false));
      REQUIRE(rightValue == NativeValue(true));
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(false));
      auto expression = MakeOrExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<OrExpression>());
      auto orExpression = expression.StaticCast<OrExpression>();
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(false));
    }
    {
      auto subexpressions = std::vector<Expression>();
      subexpressions.push_back(ConstantExpression(true));
      subexpressions.push_back(ConstantExpression(true));
      auto expression = MakeOrExpression(subexpressions.begin(),
        subexpressions.end());
      REQUIRE_NOTHROW(expression.DynamicCast<OrExpression>());
      auto orExpression = expression.StaticCast<OrExpression>();
      REQUIRE(orExpression.GetType() == BoolType());
      auto leftValue = orExpression.GetLeftExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      auto rightValue = orExpression.GetRightExpression().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(leftValue == NativeValue(true));
      REQUIRE(rightValue == NativeValue(true));
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
        auto expression = MakeOrExpression(subexpressions.begin(),
          subexpressions.end());
        REQUIRE_NOTHROW(expression.DynamicCast<OrExpression>());
        auto orExpression = expression.StaticCast<OrExpression>();
        REQUIRE(orExpression.GetType() == BoolType());
        auto traversal = static_cast<const VirtualExpression*>(&orExpression);
        for(auto k = std::size_t(0); k < i - 1; ++k) {
          auto value = ((j & (1UL << k)) != 0);
          auto expression = static_cast<const OrExpression*>(traversal);
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
