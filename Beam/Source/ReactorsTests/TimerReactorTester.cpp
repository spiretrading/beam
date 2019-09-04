#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("TimerReactorTester") {
  TEST_CASE("Test expiry") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto timer = std::shared_ptr<TriggerTimer>();
    auto timerFactory =
      [&] (time_duration duration) {
        timer = std::make_shared<TriggerTimer>();
        return timer;
      };
    auto reactor = TimerReactor<int>(timerFactory, seconds(5));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::NONE);
    timer->Trigger();
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    Trigger::set_trigger(nullptr);
  }
}
