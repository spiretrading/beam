#include "Beam/ReactorsTests/AlarmReactorTester.hpp"
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Reactors/Control.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace Beam::Routines;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void AlarmReactorTester::TestFutureThenPastExpiry() {
  TriggerTimer* timer;
  Async<void> timerToken;
  auto factory =
    [&] (time_duration duration) {
      auto localTimer = std::make_unique<TriggerTimer>();
      timer = localTimer.get();
      timerToken.GetEval().SetResult();
      return localTimer;
    };
  ReactorMonitor monitor;
  auto expiryTrigger = MakeTriggeredReactor<ptime>();
  monitor.AddEvent(expiryTrigger);
  auto timeClient = std::make_shared<IncrementalTimeClient>();
  auto alarmReactor = MakeAlarmReactor(std::move(factory), timeClient,
    expiryTrigger);
  Async<Expect<bool>> alarmToken;
  auto alarmMonitor = Do(
    [&] (const Expect<bool>& result) {
      alarmToken.GetEval().SetResult(result);
    }, get<0>(alarmReactor));
  monitor.AddReactor(alarmMonitor);
  monitor.AddEvent(get<1>(alarmReactor));
  monitor.Open();
  expiryTrigger->SetValue(timeClient->GetTime() + days(1));
  expiryTrigger->Trigger();
  timerToken.Get();
  timerToken.Reset();
  CPPUNIT_ASSERT(timer != nullptr);
  auto futureTriggerResult = alarmToken.Get();
  CPPUNIT_ASSERT(futureTriggerResult.IsValue());
  CPPUNIT_ASSERT(futureTriggerResult.Get() == false);
  alarmToken.Reset();
  timer->Trigger();
  auto futureExpiryResult = alarmToken.Get();
  CPPUNIT_ASSERT(futureExpiryResult.IsValue());
  CPPUNIT_ASSERT(futureExpiryResult.Get() == true);
}

void AlarmReactorTester::TestPastThenFutureExpiry() {
}
