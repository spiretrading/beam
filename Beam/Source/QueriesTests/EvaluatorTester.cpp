#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("Evaluator") {
  TEST_CASE("constant_expression") {
    auto intExpression = ConstantExpression(123);
    auto evaluator = Translate(intExpression);
    REQUIRE(evaluator->Eval<int>() == 123);
  }

  TEST_CASE("addition_expression") {
    auto addition = MakeAdditionExpression(ConstantExpression(123),
      ConstantExpression(321));
    auto evaluator = Translate(addition);
    REQUIRE(evaluator->Eval<int>() == 444);
  }

  TEST_CASE("parameter_expression") {
    auto equals = MakeEqualsExpression(ParameterExpression(0, BoolType()),
      ParameterExpression(1, BoolType()));
    auto evaluator = Translate(equals);
    REQUIRE(evaluator->Eval<bool>(false, false) == true);
    REQUIRE(evaluator->Eval<bool>(false, true) == false);
    REQUIRE(evaluator->Eval<bool>(true, false) == false);
    REQUIRE(evaluator->Eval<bool>(true, true) == true);
  }

  TEST_CASE("reduce_expression") {
    auto sumExpression = MakeAdditionExpression(ParameterExpression(0, IntType()),
      ParameterExpression(1, IntType()));
    auto initialValue = IntValue(0);
    auto reduceExpression = ReduceExpression(sumExpression,
      ParameterExpression(0, IntType()), initialValue);
    auto evaluator = Translate(reduceExpression);
    REQUIRE(evaluator->Eval<int>(1) == 1);
    REQUIRE(evaluator->Eval<int>(1) == 2);
    REQUIRE(evaluator->Eval<int>(2) == 4);
    REQUIRE(evaluator->Eval<int>(5) == 9);
  }

  TEST_CASE("out_of_range_parameter_expression") {
    {
      auto parameter = ParameterExpression(MAX_EVALUATOR_PARAMETERS, BoolType());
      REQUIRE_THROWS_AS(Translate(parameter), ExpressionTranslationException);
    }
    {
      auto parameter = ParameterExpression(-1, BoolType());
      REQUIRE_THROWS_AS(Translate(parameter), ExpressionTranslationException);
    }
  }

  TEST_CASE("mismatched_type_parameter_expressions") {
    auto leftExpression = MakeEqualsExpression(ParameterExpression(0, IntType()),
      ParameterExpression(1, IntType()));
    auto rightExpression = MakeEqualsExpression(
      ParameterExpression(0, DecimalType()),
      ParameterExpression(1, DecimalType()));
    auto orExpression = OrExpression(leftExpression, rightExpression);
    REQUIRE_THROWS_AS(Translate(orExpression),
      ExpressionTranslationException);
  }

  TEST_CASE("parameter_expression_gap") {
    auto parameter = ParameterExpression(1, BoolType());
    REQUIRE_THROWS_AS(Translate(parameter), ExpressionTranslationException);
  }
}
