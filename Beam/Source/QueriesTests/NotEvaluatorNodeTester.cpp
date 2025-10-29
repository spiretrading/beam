#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/NotEvaluatorNode.hpp"

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
