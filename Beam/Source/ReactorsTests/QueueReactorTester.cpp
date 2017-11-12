#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void QueueReactorTester::TestEmptyQueue() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::COMPLETE);
  AssertException<ReactorUnavailableException>(*reactor, 2,
    BaseReactor::Update::NONE);
}

void QueueReactorTester::TestImmediateException() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 1,
    BaseReactor::Update::COMPLETE_WITH_EVAL);
  AssertException<DummyException>(*reactor, 2, BaseReactor::Update::NONE);
}

void QueueReactorTester::TestSingleValue() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  queue->Push(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 10, BaseReactor::Update::EVAL, 123);
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 11);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 11, BaseReactor::Update::COMPLETE, 123);
  AssertValue(*reactor, 12, BaseReactor::Update::NONE, 123);
}

void QueueReactorTester::TestSingleValueException() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  queue->Push(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 10, BaseReactor::Update::EVAL, 123);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 11);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 11,
    BaseReactor::Update::COMPLETE_WITH_EVAL);
  AssertException<DummyException>(*reactor, 12, BaseReactor::Update::NONE);
}
