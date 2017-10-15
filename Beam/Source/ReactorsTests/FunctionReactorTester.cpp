#include "Beam/ReactorsTests/FunctionReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace boost;
using namespace std;

namespace {
  struct DummyException : std::exception {};

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
}

void FunctionReactorTester::TestNoParameters() {
  auto reactor = MakeFunctionReactor(&NoParameterFunction);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), 512);
}

void FunctionReactorTester::TestNoParametersThrow() {
  auto reactor = MakeFunctionReactor(&NoParameterFunctionThrow);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
}

void FunctionReactorTester::TestNoParametersEmpty() {
  auto reactor = MakeFunctionReactor(&NoParameterFunctionEmpty);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
}

void FunctionReactorTester::TestOneConstantParameter() {
  auto reactor = MakeFunctionReactor(&Square, MakeConstantReactor(5));
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), 25);
}

void FunctionReactorTester::TestOneParameterNoEval() {
  auto queue = std::make_shared<Queue<int>>();
  Trigger trigger;
  auto queueReactor = MakeQueueReactor(
    static_pointer_cast<QueueReader<int>>(queue), Ref(trigger));
  auto reactor = MakeFunctionReactor(&Square, queueReactor);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
}

void FunctionReactorTester::TestOneParameterWithSingleEval() {
  auto queue = std::make_shared<Queue<int>>();
  Trigger trigger;
  auto queueReactor = MakeQueueReactor(
    static_pointer_cast<QueueReader<int>>(queue), Ref(trigger));
  auto reactor = MakeFunctionReactor(&Square, queueReactor);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  queue->Push(911);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), Square(911));
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(2) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), Square(911));
  CPPUNIT_ASSERT(reactor->Commit(3) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), Square(911));
}

void FunctionReactorTester::TestOneParameterWithMultipleEvals() {
  auto queue = std::make_shared<Queue<int>>();
  Trigger trigger;
  auto queueReactor = MakeQueueReactor(
    static_pointer_cast<QueueReader<int>>(queue), Ref(trigger));
  auto reactor = MakeFunctionReactor(&Square, queueReactor);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  queue->Push(911);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), Square(911));
  queue->Push(416);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(2) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), Square(416));
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 3);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(3) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
  CPPUNIT_ASSERT(reactor->Commit(4) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
}

void FunctionReactorTester::TestOneParameterWithFilter() {
  auto queue = std::make_shared<Queue<int>>();
  Trigger trigger;
  auto queueReactor = MakeQueueReactor(
    static_pointer_cast<QueueReader<int>>(queue), Ref(trigger));
  auto reactor = MakeFunctionReactor(&FilterOdd, queueReactor);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  queue->Push(5);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  queue->Push(10);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(2) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(reactor->Eval(), 10);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 3);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(3) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
  CPPUNIT_ASSERT(reactor->Commit(4) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
}
