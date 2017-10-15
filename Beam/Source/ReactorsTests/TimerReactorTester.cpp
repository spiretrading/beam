#include "Beam/ReactorsTests/TimerReactorTester.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/DoReactor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

void TimerReactorTester::TestExpiry() {
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  Trigger trigger;
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto period = MakeConstantReactor(time_duration{seconds(5)});
  std::shared_ptr<TriggerTimer> timer;
  auto timerFactory =
    [&] (time_duration duration) {
      timer = std::make_shared<TriggerTimer>();
      return timer;
    };
  auto timerReactor = MakeTimerReactor<int>(timerFactory, period, Ref(trigger));
  timerReactor->Commit(0);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  timerReactor->Commit(1);
  timer->Trigger();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
}
