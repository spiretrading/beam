#include "Beam/ReactorsTests/TimerReactorTester.hpp"
#include <aspen/Trigger.hpp>
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

void TimerReactorTester::TestExpiry() {
  auto commits = Queue<bool>();
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
  auto reactor = Timer<int>(timerFactory, seconds(5));
  CPPUNIT_ASSERT(reactor.commit(0) == State::CONTINUE_EVALUATED);
  CPPUNIT_ASSERT(reactor.eval() == 0);
  CPPUNIT_ASSERT(reactor.commit(1) == State::NONE);
  timer->Trigger();
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(2) == State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval() == 1);
  Trigger::set_trigger(nullptr);
}
