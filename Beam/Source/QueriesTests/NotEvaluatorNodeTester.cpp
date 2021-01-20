#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/NotEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("NotEvaluatorNode") {
  TEST_CASE("constructor") {
    {
      auto evaluator =
        NotEvaluatorNode(std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(true)));
      REQUIRE(!evaluator.Eval());
    }
    {
      auto evaluator =
        NotEvaluatorNode(std::make_unique<ConstantEvaluatorNode<bool>>(
          ConstantEvaluatorNode(false)));
      REQUIRE(evaluator.Eval());
    }
  }
}
