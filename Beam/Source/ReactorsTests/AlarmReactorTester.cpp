#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("AlarmReactorTester") {
  TEST_CASE("Test expiry") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto startDate = ptime(date(2019, 5, 2), time_duration(13, 1, 12));
    auto expiryDate = ptime(date(2019, 5, 2), time_duration(13, 2, 12));
    auto timeClient = FixedTimeClient(ptime(date(2019, 5, 2),
      time_duration(13, 1, 12)));
    auto timer = std::shared_ptr<TriggerTimer>();
    auto timerFactory =
      [&] (time_duration duration) {
        timer = std::make_shared<TriggerTimer>();
        return timer;
      };
    auto reactor = AlarmReactor(&timeClient, timerFactory, expiryDate);
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
