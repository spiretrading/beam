#include "Beam/QueriesTests/ParameterEvaluatorNodeTester.hpp"
#include "Beam/Queries/ParameterEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void ParameterEvaluatorNodeTester::TestInt() {
  ParameterEvaluatorNode<int> parameter(0);
  int value = 123;
  const void* valuePtr = &value;
  parameter.SetParameter(&valuePtr);
  CPPUNIT_ASSERT(parameter.Eval() == 123);
}
