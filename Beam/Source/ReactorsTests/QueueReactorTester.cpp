#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

namespace {
  struct DummyException : std::exception {};
}

void QueueReactorTester::TestEmptyQueue() {
  auto queue = std::make_shared<Queue<int>>();
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  Trigger trigger;
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(9) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(11) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(reactor->IsComplete());
}

void QueueReactorTester::TestImmediateException() {
  auto queue = std::make_shared<Queue<int>>();
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  Trigger trigger;
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(9) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(11) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), DummyException);
  CPPUNIT_ASSERT(reactor->IsComplete());
}

void QueueReactorTester::TestSingleValue() {
  auto queue = std::make_shared<Queue<int>>();
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  Trigger trigger;
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  queue->Push(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(9) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->Eval() == 123);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::EVAL);
  CPPUNIT_ASSERT(reactor->Eval() == 123);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT(reactor->Commit(11) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->Eval() == 123);
  CPPUNIT_ASSERT(!reactor->IsComplete());
}
