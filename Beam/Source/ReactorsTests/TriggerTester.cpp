#include "Beam/ReactorsTests/TriggerTester.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Routines/Async.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace Beam::Routines;
using namespace std;

void TriggerTester::TestOpenTrigger() {
  ReactorMonitor monitor;
  Trigger trigger(monitor);
  monitor.Open();
  Async<void> token;
  trigger.Do(
    [&] {
      token.GetEval().SetResult();
    });
  token.Get();
}
