module;
#include "Prelude.hpp"
#include <doctest/doctest.h>

module Beam;

using namespace Beam;

TEST_SUITE("AndEvaluatorNode") {
  TEST_CASE("constructor") {
    {
      auto evaluator = AndEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(false)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(false)));
      REQUIRE(!evaluator.eval());
    }
    {
      auto evaluator = AndEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(false)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(true)));
      REQUIRE(!evaluator.eval());
    }
    {
      auto evaluator = AndEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(true)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(false)));
      REQUIRE(!evaluator.eval());
    }
    {
      auto evaluator = AndEvaluatorNode(
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(true)),
        std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(true)));
      REQUIRE(evaluator.eval());
    }
  }
}
