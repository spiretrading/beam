#include <vector>
#include <Aspen/CommitFlag.hpp>
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
    auto flag = CommitFlag();
    flag.set_trigger(&trigger);
    auto scope = CommitFlagScope(flag);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    flag.clear();
    queue->close();
    commits.pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("immediate_exception") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    auto flag = CommitFlag();
    flag.set_trigger(&trigger);
    auto scope = CommitFlagScope(flag);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    flag.clear();
    queue->close(std::runtime_error("Broken."));
    commits.pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS_MESSAGE(reactor.eval(), std::runtime_error, "Broken.");
  }

  TEST_CASE("single_value") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    auto flag = CommitFlag();
    flag.set_trigger(&trigger);
    auto scope = CommitFlagScope(flag);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    flag.clear();
    queue->push(123);
    queue->close();
    auto values = std::vector<int>();
    auto sequence = 1;
    while(true) {
      auto state = reactor.commit(sequence);
      ++sequence;
      if(has_evaluation(state)) {
        values.push_back(reactor.eval());
      }
      if(is_complete(state)) {
        break;
      }
      if(!has_continuation(state)) {
        flag.clear();
        commits.pop();
      }
    }
    REQUIRE(values == std::vector{123});
  }

  TEST_CASE("single_value_exception") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    auto flag = CommitFlag();
    flag.set_trigger(&trigger);
    auto scope = CommitFlagScope(flag);
    auto queue = std::make_shared<Beam::Queue<int>>();
    auto reactor = QueueReactor(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    flag.clear();
    queue->push(123);
    queue->close(std::runtime_error("Broken."));
    auto has_exception = false;
    auto values = std::vector<int>();
    auto sequence = 1;
    while(true) {
      auto state = reactor.commit(sequence);
      ++sequence;
      if(has_evaluation(state)) {
        if(is_complete(state)) {
          REQUIRE_THROWS_AS_MESSAGE(
            reactor.eval(), std::runtime_error, "Broken.");
          has_exception = true;
        } else {
          values.push_back(reactor.eval());
        }
      }
      if(is_complete(state)) {
        break;
      }
      if(!has_continuation(state)) {
        flag.clear();
        commits.pop();
      }
    }
    REQUIRE(values == std::vector{123});
    REQUIRE(has_exception);
  }
}
