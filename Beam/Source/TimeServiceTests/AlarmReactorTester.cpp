#include <Aspen/Constant.hpp>
#include <Aspen/Queue.hpp>
#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/TimeService/AlarmReactor.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("AlarmReactor") {
  TEST_CASE("expiry") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Aspen::Trigger([&] {
      commits.push(true);
    });
    Aspen::Trigger::set_trigger(trigger);
    auto start_time = time_from_string("2020-01-01 12:00:00");
    auto time_client = FixedTimeClient(start_time);
    auto expiry = time_from_string("2020-02-01 12:00:00");
    auto timer = std::shared_ptr<TriggerTimer>();
    auto timer_factory = [&] (auto duration) {
      timer = std::make_shared<TriggerTimer>();
      return timer;
    };
    auto reactor = alarm_reactor(&time_client, timer_factory, expiry);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::NONE);
    timer->trigger();
    commits.pop();
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    Trigger::set_trigger(nullptr);
  }
}
