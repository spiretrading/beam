#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/TimeService/TimerReactor.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("TimerReactor") {
  TEST_CASE("expiry") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto timer = std::shared_ptr<TriggerTimer>();
    auto timer_factory = [&] (auto duration) {
      timer = std::make_shared<TriggerTimer>();
      return timer;
    };
    auto reactor = timer_reactor<int>(timer_factory, seconds(5));
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
