#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void QueueReactorTester::TestEmptyQueue() {
  Trigger trigger;
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::COMPLETE);
  AssertException<ReactorUnavailableException>(*reactor, 2,
    BaseReactor::Update::NONE, true);
}

void QueueReactorTester::TestImmediateException() {
  Trigger trigger;
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 1, BaseReactor::Update::EVAL, true);
  AssertException<DummyException>(*reactor, 2, BaseReactor::Update::NONE, true);
}

void QueueReactorTester::TestSingleValue() {
  Trigger trigger;
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
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
  AssertValue(*reactor, 12, BaseReactor::Update::NONE, 123, true);
}

void QueueReactorTester::TestSingleValueException() {
  Trigger trigger;
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  queue->Push(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 10, BaseReactor::Update::EVAL, 123);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 11);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 11, BaseReactor::Update::EVAL,
    true);
  AssertException<DummyException>(*reactor, 12, BaseReactor::Update::NONE,
    true);
}
