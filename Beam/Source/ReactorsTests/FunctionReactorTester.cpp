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
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 512, true);
  AssertValue(*reactor, 1, BaseReactor::Update::NONE, 512, true);
}

void FunctionReactorTester::TestNoParametersThrow() {
  auto reactor = MakeFunctionReactor(&NoParameterFunctionThrow);
  AssertException<DummyException>(*reactor, 0, BaseReactor::Update::EVAL, true);
  AssertException<DummyException>(*reactor, 1, BaseReactor::Update::NONE, true);
}

void FunctionReactorTester::TestNoParametersEmpty() {
  auto reactor = MakeFunctionReactor(&NoParameterFunctionEmpty);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::COMPLETE, true);
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE, true);
}

void FunctionReactorTester::TestOneConstantParameter() {
  auto reactor = MakeFunctionReactor(&Square, MakeConstantReactor(5));
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 25, true);
  AssertValue(*reactor, 1, BaseReactor::Update::NONE, 25, true);
}

void FunctionReactorTester::TestOneParameterNoEval() {
  Trigger trigger;
  auto p1 = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeFunctionReactor(&Square, p1);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  p1->SetComplete();
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::COMPLETE, true);
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE, true);
}

void FunctionReactorTester::TestOneParameterWithSingleEval() {
  Trigger trigger;
  auto p1 = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeFunctionReactor(&Square, p1);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  p1->Update(911);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, Square(911), false);
  p1->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::COMPLETE, Square(911), true);
  AssertValue(*reactor, 2, BaseReactor::Update::NONE, Square(911), true);
}

void FunctionReactorTester::TestOneParameterWithMultipleEvals() {
  Trigger trigger;
  auto p1 = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeFunctionReactor(&Square, p1);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  p1->Update(911);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, Square(911), false);
  p1->Update(416);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, Square(416), false);
  p1->SetComplete(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 2, BaseReactor::Update::EVAL, true);
  AssertException<DummyException>(*reactor, 3, BaseReactor::Update::NONE, true);
}

void FunctionReactorTester::TestOneParameterWithFilter() {
  Trigger trigger;
  auto p1 = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeFunctionReactor(&FilterOdd, p1);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  p1->Update(5);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE, false);
  p1->Update(10);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, 10, false);
  p1->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::COMPLETE, 10, true);
  AssertValue(*reactor, 3, BaseReactor::Update::NONE, 10, true);
}

void FunctionReactorTester::TestTwoConstantParameters() {
  Trigger trigger;
  auto reactor = MakeFunctionReactor(&JoinFunction,
    MakeConstantReactor(100), MakeConstantReactor(string{"a"}));
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL,
    make_tuple(100, "a"), true);
  AssertValue(*reactor, 1, BaseReactor::Update::NONE,
    make_tuple(100, "a"), true);
}
