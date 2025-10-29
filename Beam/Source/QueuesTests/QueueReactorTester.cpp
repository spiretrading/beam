#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReactor.hpp"

using namespace Aspen;
using namespace Beam;

TEST_SUITE("QueueReactor") {
  TEST_CASE("empty") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->close();
    commits.pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("immediate_exception") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->close(std::runtime_error("Broken."));
    commits.pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS_MESSAGE(reactor.eval(), std::runtime_error, "Broken.");
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("single_value") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(123);
    queue->close();
    commits.pop();
    commits.pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("single_value_exception") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(123);
    queue->close(std::runtime_error("Broken."));
    commits.pop();
    commits.pop();
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS_MESSAGE(reactor.eval(), std::runtime_error, "Broken.");
    Trigger::set_trigger(nullptr);
  }
}
