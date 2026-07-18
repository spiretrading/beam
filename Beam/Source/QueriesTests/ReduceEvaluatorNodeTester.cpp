module;
#include "Prelude.hpp"
#include <doctest/doctest.h>

module Beam;

using namespace Beam;

TEST_SUITE("ReduceEvaluatorNode") {
  TEST_CASE("reduce_sum") {
    auto sum =
      ParameterExpression(0, typeid(int)) + ParameterExpression(1, typeid(int));
    auto reducer = translate(sum);
    auto parameter = std::make_unique<ConstantEvaluatorNode<int>>(1);
    auto evaluator =
      ReduceEvaluatorNode(std::move(reducer), std::move(parameter), 0);
    REQUIRE(evaluator.eval() == 1);
    REQUIRE(evaluator.eval() == 2);
    REQUIRE(evaluator.eval() == 3);
    REQUIRE(evaluator.eval() == 4);
  }
}
