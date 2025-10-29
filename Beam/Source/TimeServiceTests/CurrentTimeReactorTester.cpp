#include <Aspen/Constant.hpp>
#include <Aspen/Queue.hpp>
#include <Aspen/Shared.hpp>
#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/TimeService/CurrentTimeReactor.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("CurrentTimeReactor") {
  TEST_CASE("current_time_with_fixed_client") {
    auto time = time_from_string("2025-01-15 10:30:00");
    auto client = FixedTimeClient(time);
    auto reactor = current_time_reactor(&client);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == time);
  }

  TEST_CASE("current_time_with_pulse") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Aspen::Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto time = time_from_string("2025-01-15 10:30:00");
    auto client = FixedTimeClient(time);
    auto pulse = Aspen::Shared<Aspen::Queue<int>>();
    auto reactor = current_time_reactor(&client, pulse);
    REQUIRE(reactor.commit(0) == State::NONE);
    pulse->push(1);
    commits.pop();
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == time);
    auto updated_time = time_from_string("2025-01-15 11:00:00");
    client.set(updated_time);
    pulse->push(2);
    commits.pop();
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == updated_time);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("current_time_with_constant_pulse") {
    auto time = time_from_string("2025-01-15 14:00:00");
    auto client = FixedTimeClient(time);
    auto reactor = current_time_reactor(&client, 0);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == time);
  }

  TEST_CASE("current_time_updates_with_client") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Aspen::Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto initial_time = time_from_string("2025-01-15 09:00:00");
    auto client = FixedTimeClient(initial_time);
    auto pulse = Aspen::Shared<Aspen::Queue<int>>();
    auto reactor = current_time_reactor(&client, pulse);
    REQUIRE(reactor.commit(0) == State::NONE);
    pulse->push(0);
    commits.pop();
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == initial_time);
    auto updated_time = time_from_string("2025-01-15 15:00:00");
    client.set(updated_time);
    pulse->push(1);
    commits.pop();
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == updated_time);
    auto final_time = time_from_string("2025-01-15 18:00:00");
    client.set(final_time);
    pulse->push(2);
    commits.pop();
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == final_time);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("current_time_with_unique_ptr") {
    auto time = time_from_string("2025-01-15 12:00:00");
    auto client = std::make_unique<FixedTimeClient>(time);
    auto reactor = current_time_reactor(std::move(client));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == time);
  }

  TEST_CASE("current_time_no_arguments") {
    auto reactor = current_time_reactor();
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    auto result = reactor.eval();
    REQUIRE(!result.is_special());
  }
}
