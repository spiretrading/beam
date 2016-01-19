#include "Beam/QueriesTests/OrEvaluatorNodeTester.hpp"
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/OrEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void OrEvaluatorNodeTester::TestConstructor() {
  {
    OrEvaluatorNode evaluator(std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(false)),
      std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(false)));
    CPPUNIT_ASSERT(evaluator.Eval() == false);
  }
  {
    OrEvaluatorNode evaluator(std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(false)),
      std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(true)));
    CPPUNIT_ASSERT(evaluator.Eval() == true);
  }
  {
    OrEvaluatorNode evaluator(std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(true)),
      std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(false)));
    CPPUNIT_ASSERT(evaluator.Eval() == true);
  }
  {
    OrEvaluatorNode evaluator(std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(true)),
      std::make_unique<ConstantEvaluatorNode<bool>>(
      MakeConstantEvaluatorNode(true)));
    CPPUNIT_ASSERT(evaluator.Eval() == true);
  }
}
