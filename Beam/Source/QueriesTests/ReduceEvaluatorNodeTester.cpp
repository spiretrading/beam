#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/ReduceEvaluatorNode.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("ReduceEvaluatorNode") {
  TEST_CASE("reduce_sum") {
    auto sumExpression = MakeAdditionExpression(
      ParameterExpression(0, IntType()), ParameterExpression(1, IntType()));
    auto reducer = Translate(sumExpression);
    auto parameter = std::make_unique<ConstantEvaluatorNode<int>>(1);
    auto reduceEvaluator = ReduceEvaluatorNode(std::move(reducer),
      std::move(parameter), 0);
    REQUIRE(reduceEvaluator.Eval() == 1);
    REQUIRE(reduceEvaluator.Eval() == 2);
    REQUIRE(reduceEvaluator.Eval() == 3);
    REQUIRE(reduceEvaluator.Eval() == 4);
  }
}
