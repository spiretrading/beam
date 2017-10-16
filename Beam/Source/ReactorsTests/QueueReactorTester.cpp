#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void QueueReactorTester::TestEmptyQueue() {
  Trigger trigger;
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  queue->Break();
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
}

void QueueReactorTester::TestImmediateException() {
  Trigger trigger;
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->Commit(0) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
}

void QueueReactorTester::TestSingleValue() {
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  Trigger trigger;
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
  queue->Push(123);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 123);
  trigger.SignalUpdate(Store(dummySequenceNumber));
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 10, BaseReactor::Update::NONE, 123);
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 11);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 11, BaseReactor::Update::COMPLETE, 123, true);
  AssertValue(*reactor, 12, BaseReactor::Update::NONE, 123, true);
}

void QueueReactorTester::TestSingleValueException() {
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  Trigger trigger;
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
  queue->Push(123);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 123);
  trigger.SignalUpdate(Store(dummySequenceNumber));
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 10, BaseReactor::Update::NONE, 123);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 11);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 11, BaseReactor::Update::EVAL,
    true);
  AssertException<DummyException>(*reactor, 12, BaseReactor::Update::NONE,
    true);
}
