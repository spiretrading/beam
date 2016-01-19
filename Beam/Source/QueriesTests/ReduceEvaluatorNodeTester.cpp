#include "Beam/QueriesTests/ReduceEvaluatorNodeTester.hpp"
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/ReduceEvaluatorNode.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void ReduceEvaluatorNodeTester::TestReduceSum() {
  auto sumExpression = MakeAdditionExpression(ParameterExpression(0, IntType()),
    ParameterExpression(1, IntType()));
  std::unique_ptr<Evaluator> reducer = Translate(sumExpression);
  std::unique_ptr<EvaluatorNode<int>> parameter =
    std::make_unique<ConstantEvaluatorNode<int>>(1);
  ReduceEvaluatorNode<int> reduceEvaluator(std::move(reducer),
    std::move(parameter), 0);
  CPPUNIT_ASSERT(reduceEvaluator.Eval() == 1);
  CPPUNIT_ASSERT(reduceEvaluator.Eval() == 2);
  CPPUNIT_ASSERT(reduceEvaluator.Eval() == 3);
  CPPUNIT_ASSERT(reduceEvaluator.Eval() == 4);
}
