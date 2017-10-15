#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

namespace {
  struct DummyException : std::exception {};

  void AssertValue(Reactor<int>& reactor, int sequenceNumber,
      BaseReactor::Update update, int expectedValue, bool isComplete = false) {
    CPPUNIT_ASSERT(reactor.Commit(sequenceNumber) == update);
    CPPUNIT_ASSERT(reactor.IsInitialized());
    CPPUNIT_ASSERT(reactor.IsComplete() == isComplete);
    CPPUNIT_ASSERT_EQUAL(reactor.Eval(), expectedValue);
  }

  template<typename T>
  void AssertException(Reactor<int>& reactor, int sequenceNumber,
      BaseReactor::Update update, bool isComplete = false) {
    CPPUNIT_ASSERT(reactor.Commit(sequenceNumber) == update);
    try {
      reactor.Eval();
      CPPUNIT_FAIL("Expected exception not thrown.");
    } catch(const T&) {
    } catch(const std::exception&) {
      CPPUNIT_FAIL("Expected exception not thrown.");
    }
    CPPUNIT_ASSERT(reactor.IsComplete() == isComplete);
  }
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
  CPPUNIT_ASSERT(!reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE);
  AssertException<ReactorUnavailableException>(*reactor, 9,
    BaseReactor::Update::NONE);
  AssertException<ReactorUnavailableException>(*reactor, 10,
    BaseReactor::Update::COMPLETE, true);
  AssertException<ReactorUnavailableException>(*reactor, 10,
    BaseReactor::Update::COMPLETE, true);
  AssertException<ReactorUnavailableException>(*reactor, 11,
    BaseReactor::Update::NONE, true);
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
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE);
  AssertException<ReactorUnavailableException>(*reactor, 9,
    BaseReactor::Update::NONE);
  AssertException<DummyException>(*reactor, 10, BaseReactor::Update::EVAL,
    true);
  AssertException<DummyException>(*reactor, 10, BaseReactor::Update::EVAL,
    true);
  AssertException<DummyException>(*reactor, 11, BaseReactor::Update::NONE,
    true);
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
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE);
  AssertException<ReactorUnavailableException>(*reactor, 9,
    BaseReactor::Update::NONE);
  AssertValue(*reactor, 10, BaseReactor::Update::EVAL, 123);
  AssertValue(*reactor, 10, BaseReactor::Update::EVAL, 123);
  trigger.SignalUpdate(Store(dummySequenceNumber));
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 11);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 11, BaseReactor::Update::NONE, 123);
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 12);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 12, BaseReactor::Update::COMPLETE, 123, true);
  AssertValue(*reactor, 13, BaseReactor::Update::NONE, 123, true);
}

void QueueReactorTester::TestSingleValueException() {
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
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE);
  AssertException<ReactorUnavailableException>(*reactor, 9,
    BaseReactor::Update::NONE);
  AssertValue(*reactor, 10, BaseReactor::Update::EVAL, 123);
  AssertValue(*reactor, 10, BaseReactor::Update::EVAL, 123);
  trigger.SignalUpdate(Store(dummySequenceNumber));
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 11);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 11, BaseReactor::Update::NONE, 123);
  queue->Break(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 12);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 12, BaseReactor::Update::EVAL,
    true);
  AssertException<DummyException>(*reactor, 13, BaseReactor::Update::NONE,
    true);
}
