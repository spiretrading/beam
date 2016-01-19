#include "Beam/QueriesTests/ConversionEvaluatorNodeTester.hpp"
#include <string>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/ConversionEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void ConversionEvaluatorNodeTester::TestCastIntToDouble() {
  auto value = make_unique<ConstantEvaluatorNode<int>>(123);
  auto node = MakeCastEvaluatorNode<int, double>(std::move(value));
  CPPUNIT_ASSERT(node->Eval() == 123.0);
}

void ConversionEvaluatorNodeTester::TestConvertCharArrayToString() {
  auto value = make_unique<ConstantEvaluatorNode<const char*>>("hello world");
  auto node = MakeConstructEvaluatorNode<const char*, string>(std::move(value));
  CPPUNIT_ASSERT(node->Eval() == string{"hello world"});
}
