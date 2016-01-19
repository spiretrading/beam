#include "Beam/QueriesTests/FunctionEvaluatorNodeTester.hpp"
#include <string>
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/FunctionEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void FunctionEvaluatorNodeTester::TestEmptyFunction() {
  struct Function {
    int operator ()() const {
      return 123;
    }
  };
  FunctionEvaluatorNode<Function> function{Function()};
  CPPUNIT_ASSERT(function.Eval() == 123);
}

void FunctionEvaluatorNodeTester::TestUnaryFunction() {
  struct Function {
    string operator ()(string value) const {
      CPPUNIT_ASSERT(value == "hello ");
      return value + "world";
    }
  };
  FunctionEvaluatorNode<Function> function{Function(),
    std::make_unique<ConstantEvaluatorNode<string>>(
    MakeConstantEvaluatorNode<string>("hello "))};
  CPPUNIT_ASSERT(function.Eval() == "hello world");
}

void FunctionEvaluatorNodeTester::TestBinaryFunction() {
  struct Function {
    double operator ()(int lhs, double rhs) const {
      CPPUNIT_ASSERT(lhs == 2);
      CPPUNIT_ASSERT(rhs == 3.14);
      return lhs * rhs;
    }
  };
  FunctionEvaluatorNode<Function> function{Function(),
    std::make_unique<ConstantEvaluatorNode<int>>(
    MakeConstantEvaluatorNode(2)),
    std::make_unique<ConstantEvaluatorNode<double>>(
    MakeConstantEvaluatorNode(3.14))};
  CPPUNIT_ASSERT(function.Eval() == 6.28);
}
