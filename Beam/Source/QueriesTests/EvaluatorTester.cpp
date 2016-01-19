#include "Beam/QueriesTests/EvaluatorTester.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void EvaluatorTester::TestConstantExpression() {
  ConstantExpression intExpression = MakeConstantExpression(123);
  unique_ptr<Evaluator> evaluator = Translate(intExpression);
  CPPUNIT_ASSERT(evaluator->Eval<int>() == 123);
}

void EvaluatorTester::TestAdditionExpression() {
  auto addition = MakeAdditionExpression(MakeConstantExpression(123),
    MakeConstantExpression(321));
  unique_ptr<Evaluator> evaluator = Translate(addition);
  CPPUNIT_ASSERT(evaluator->Eval<int>() == 444);
}

void EvaluatorTester::TestParameterExpression() {
  auto equals = MakeEqualsExpression(ParameterExpression(0, BoolType()),
    ParameterExpression(1, BoolType()));
  unique_ptr<Evaluator> evaluator = Translate(equals);
  CPPUNIT_ASSERT(evaluator->Eval<bool>(false, false) == true);
  CPPUNIT_ASSERT(evaluator->Eval<bool>(false, true) == false);
  CPPUNIT_ASSERT(evaluator->Eval<bool>(true, false) == false);
  CPPUNIT_ASSERT(evaluator->Eval<bool>(true, true) == true);
}

void EvaluatorTester::TestReduceExpression() {
  auto sumExpression = MakeAdditionExpression(ParameterExpression(0, IntType()),
    ParameterExpression(1, IntType()));
  IntValue initialValue(0);
  ReduceExpression reduceExpression(sumExpression,
    ParameterExpression(0, IntType()), initialValue);
  std::unique_ptr<Evaluator> evaluator = Translate(reduceExpression);
  CPPUNIT_ASSERT(evaluator->Eval<int>(1) == 1);
  CPPUNIT_ASSERT(evaluator->Eval<int>(1) == 2);
  CPPUNIT_ASSERT(evaluator->Eval<int>(2) == 4);
  CPPUNIT_ASSERT(evaluator->Eval<int>(5) == 9);
}

void EvaluatorTester::TestOutOfRangeParameterExpression() {
  {
    auto parameter = ParameterExpression(MAX_EVALUATOR_PARAMETERS, BoolType());
    CPPUNIT_ASSERT_THROW(Translate(parameter), ExpressionTranslationException);
  }
  {
    auto parameter = ParameterExpression(-1, BoolType());
    CPPUNIT_ASSERT_THROW(Translate(parameter), ExpressionTranslationException);
  }
}

void EvaluatorTester::TestMismatchedTypeParameterExpressions() {
  auto leftExpression = MakeEqualsExpression(ParameterExpression(0, IntType()),
    ParameterExpression(1, IntType()));
  auto rightExpression = MakeEqualsExpression(
    ParameterExpression(0, DecimalType()),
    ParameterExpression(1, DecimalType()));
  auto orExpression = OrExpression(leftExpression, rightExpression);
  CPPUNIT_ASSERT_THROW(Translate(orExpression),
    ExpressionTranslationException);
}

void EvaluatorTester::TestParameterExpressionGap() {
  auto parameter = ParameterExpression(1, BoolType());
  CPPUNIT_ASSERT_THROW(Translate(parameter), ExpressionTranslationException);
}
