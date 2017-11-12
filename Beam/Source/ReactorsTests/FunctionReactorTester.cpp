#include "Beam/ReactorsTests/FunctionReactorTester.hpp"
#include <tuple>
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace boost;
using namespace std;

namespace {
  int NoParameterFunction() {
    return 512;
  }

  int NoParameterFunctionThrow() {
    throw DummyException{};
  }

  optional<int> NoParameterFunctionEmpty() {
    return none;
  }

  int Square(int x) {
    return x * x;
  }

  optional<int> FilterOdd(int value) {
    if(value % 2 == 0) {
      return value;
    }
    return none;
  }

  std::tuple<int, string> JoinFunction(int a, const string& b) {
    return make_tuple(a, b);
  }
}

void FunctionReactorTester::TestNoParameters() {
  auto reactor = MakeFunctionReactor(&NoParameterFunction);
  AssertValue(*reactor, 0, BaseReactor::Update::COMPLETE_WITH_EVAL, 512);
  AssertValue(*reactor, 1, BaseReactor::Update::NONE, 512);
}

void FunctionReactorTester::TestNoParametersThrow() {
  auto reactor = MakeFunctionReactor(&NoParameterFunctionThrow);
  AssertException<DummyException>(*reactor, 0,
    BaseReactor::Update::COMPLETE_WITH_EVAL);
  AssertException<DummyException>(*reactor, 1, BaseReactor::Update::NONE);
}

void FunctionReactorTester::TestNoParametersEmpty() {
  auto reactor = MakeFunctionReactor(&NoParameterFunctionEmpty);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::COMPLETE);
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE);
}

void FunctionReactorTester::TestOneConstantParameter() {
  auto reactor = MakeFunctionReactor(&Square, MakeConstantReactor(5));
  AssertValue(*reactor, 0, BaseReactor::Update::COMPLETE_WITH_EVAL, 25);
  AssertValue(*reactor, 1, BaseReactor::Update::NONE, 25);
}

void FunctionReactorTester::TestOneParameterNoEval() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto p1 = MakeBasicReactor<int>();
  auto reactor = MakeFunctionReactor(&Square, p1);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  p1->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::COMPLETE);
  AssertException<ReactorUnavailableException>(*reactor, 2,
    BaseReactor::Update::NONE);
}

void FunctionReactorTester::TestOneParameterWithSingleEval() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto p1 = MakeBasicReactor<int>();
  auto reactor = MakeFunctionReactor(&Square, p1);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  p1->Update(911);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, Square(911));
  p1->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::COMPLETE, Square(911));
  AssertValue(*reactor, 3, BaseReactor::Update::NONE, Square(911));
}

void FunctionReactorTester::TestOneParameterWithMultipleEvals() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto p1 = MakeBasicReactor<int>();
  auto reactor = MakeFunctionReactor(&Square, p1);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  p1->Update(911);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, Square(911));
  p1->Update(416);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::EVAL, Square(416));
  p1->SetComplete(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 3);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 3,
    BaseReactor::Update::COMPLETE_WITH_EVAL);
  AssertException<DummyException>(*reactor, 4, BaseReactor::Update::NONE);
}

void FunctionReactorTester::TestOneParameterWithFilter() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto p1 = MakeBasicReactor<int>();
  auto reactor = MakeFunctionReactor(&FilterOdd, p1);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  p1->Update(5);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE);
  p1->Update(10);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::EVAL, 10);
  p1->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 3);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 3, BaseReactor::Update::COMPLETE, 10);
  AssertValue(*reactor, 4, BaseReactor::Update::NONE, 10);
}

void FunctionReactorTester::TestTwoConstantParameters() {
  auto reactor = MakeFunctionReactor(&JoinFunction, MakeConstantReactor(100),
    MakeConstantReactor(string{"a"}));
  AssertValue(*reactor, 0, BaseReactor::Update::COMPLETE_WITH_EVAL,
    make_tuple(100, "a"));
  AssertValue(*reactor, 1, BaseReactor::Update::NONE, make_tuple(100, "a"));
}

void FunctionReactorTester::TestVoidFunction() {}
