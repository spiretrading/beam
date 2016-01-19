#include "Beam/QueriesTests/ConstantEvaluatorNodeTester.hpp"
#include <string>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void ConstantEvaluatorNodeTester::TestMakeConstantEvaluatorNode() {
  auto intNode = MakeConstantEvaluatorNode(123);
  CPPUNIT_ASSERT(intNode.Eval() == 123);
  auto stringNode = MakeConstantEvaluatorNode<string>("hello world");
  CPPUNIT_ASSERT(stringNode.Eval() == "hello world");
}

void ConstantEvaluatorNodeTester::TestInt() {
  ConstantEvaluatorNode<int> constant(123);
  CPPUNIT_ASSERT(constant.Eval() == 123);
}

void ConstantEvaluatorNodeTester::TestDecimal() {
  ConstantEvaluatorNode<double> constant(3.14);
  CPPUNIT_ASSERT(constant.Eval() == 3.14);
}

void ConstantEvaluatorNodeTester::TestString() {
  ConstantEvaluatorNode<string> constant("hello world");
  CPPUNIT_ASSERT(constant.Eval() == "hello world");
}
