#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/OrEvaluatorNode.hpp"

using namespace Beam;

TEST_SUITE("OrEvaluatorNode") {
  TEST_CASE("constructor") {
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(false),
        std::make_unique<ConstantEvaluatorNode<bool>>(false));
      REQUIRE(!evaluator.eval());
    }
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(false),
        std::make_unique<ConstantEvaluatorNode<bool>>(true));
      REQUIRE(evaluator.eval());
    }
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(true),
        std::make_unique<ConstantEvaluatorNode<bool>>(false));
      REQUIRE(evaluator.eval());
    }
    {
      auto evaluator = OrEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(true),
        std::make_unique<ConstantEvaluatorNode<bool>>(true));
      REQUIRE(evaluator.eval());
    }
  }
}
