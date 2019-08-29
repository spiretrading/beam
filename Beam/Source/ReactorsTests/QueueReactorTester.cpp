#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;

void QueueReactorTester::TestEmptyQueue() {
  auto commits = Queue<bool>();
  auto trigger = Trigger(
    [&] {
      commits.Push(true);
    });
  Trigger::set_trigger(trigger);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = QueueReactor(queue);
  CPPUNIT_ASSERT(reactor.commit(0) == State::EMPTY);
  queue->Break();
  commits.Top();
  CPPUNIT_ASSERT(reactor.commit(1) == State::COMPLETE_EMPTY);
  Trigger::set_trigger(nullptr);
}

void QueueReactorTester::TestImmediateException() {
  auto commits = Queue<bool>();
  auto trigger = Trigger(
    [&] {
      commits.Push(true);
    });
  Trigger::set_trigger(trigger);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = QueueReactor(queue);
  CPPUNIT_ASSERT(reactor.commit(0) == State::EMPTY);
  queue->Break(std::runtime_error("Broken."));
  commits.Top();
  CPPUNIT_ASSERT(reactor.commit(1) == State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT_THROW_MESSAGE("Broken.", reactor.eval(), std::runtime_error);
  Trigger::set_trigger(nullptr);
}

void QueueReactorTester::TestSingleValue() {
  auto commits = Queue<bool>();
  auto trigger = Trigger(
    [&] {
      commits.Push(true);
    });
  Trigger::set_trigger(trigger);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = QueueReactor(queue);
  CPPUNIT_ASSERT(reactor.commit(0) == State::EMPTY);
  queue->Push(123);
  queue->Break();
  commits.Top();
  commits.Pop();
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(1) == State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(reactor.eval() == 123);
  Trigger::set_trigger(nullptr);
}

void QueueReactorTester::TestSingleValueException() {
  auto commits = Queue<bool>();
  auto trigger = Trigger(
    [&] {
      commits.Push(true);
    });
  Trigger::set_trigger(trigger);
  auto queue = std::make_shared<Queue<int>>();
  auto reactor = QueueReactor(queue);
  CPPUNIT_ASSERT(reactor.commit(0) == State::EMPTY);
  queue->Push(123);
  queue->Break(std::runtime_error("Broken."));
  commits.Top();
  commits.Pop();
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(1) == State::CONTINUE_EVALUATED);
  CPPUNIT_ASSERT(reactor.eval() == 123);
  CPPUNIT_ASSERT(reactor.commit(2) == State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT_THROW_MESSAGE("Broken.", reactor.eval(), std::runtime_error);
  Trigger::set_trigger(nullptr);
}
