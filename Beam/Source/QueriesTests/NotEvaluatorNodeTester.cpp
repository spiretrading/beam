module;
#include "Prelude.hpp"
#include <doctest/doctest.h>

module Beam;

using namespace Beam;

TEST_SUITE("NotEvaluatorNode") {
  TEST_CASE("constructor") {
    {
      auto evaluator =
        NotEvaluatorNode(std::make_unique<ConstantEvaluatorNode<bool>>(true));
      REQUIRE(!evaluator.eval());
    }
    {
      auto evaluator =
        NotEvaluatorNode(std::make_unique<ConstantEvaluatorNode<bool>>(false));
      REQUIRE(evaluator.eval());
    }
  }
}
