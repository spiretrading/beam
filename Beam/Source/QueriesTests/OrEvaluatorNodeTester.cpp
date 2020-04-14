#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/OrEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("OrEvaluatorNode") {
  TEST_CASE("constructor") {
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(false)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(false)));
      REQUIRE(!evaluator.Eval());
    }
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(false)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(true)));
      REQUIRE(evaluator.Eval());
    }
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(true)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(false)));
      REQUIRE(evaluator.Eval());
    }
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(true)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
        ConstantEvaluatorNode(true)));
      REQUIRE(evaluator.Eval());
    }
  }
}
