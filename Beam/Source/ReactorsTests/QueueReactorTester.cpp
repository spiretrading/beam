#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Reactors;

TEST_SUITE("QueueReactorTester") {
  TEST_CASE("Test empty queue.") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->Break();
    commits.Top();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
    Trigger::set_trigger(nullptr);
  }
  TEST_CASE("Test immediate exception.") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->Break(std::runtime_error("Broken."));
    commits.Top();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS_MESSAGE(reactor.eval(), std::runtime_error, "Broken.");
    Trigger::set_trigger(nullptr);
  }
  TEST_CASE("Test single value.") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->Push(123);
    queue->Break();
    commits.Top();
    commits.Pop();
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    Trigger::set_trigger(nullptr);
  }
  TEST_CASE("Test single value exception.") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->Push(123);
    queue->Break(std::runtime_error("Broken."));
    commits.Top();
    commits.Pop();
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS_MESSAGE(reactor.eval(), std::runtime_error, "Broken.");
    Trigger::set_trigger(nullptr);
  }
}
