#include "Beam/ReactorsTests/ChainReactorTester.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/Control.hpp"
#include "Beam/Reactors/ChainReactor.hpp"
#include "Beam/Reactors/RangeReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Routines/Async.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace Beam::Routines;
using namespace std;

void ChainReactorTester::TestRange() {
  auto initialRange = MakeRangeReactor(
    MakeConstantReactor(0), MakeConstantReactor(10));
  auto continuationRange = MakeRangeReactor(
    MakeConstantReactor(11), MakeConstantReactor(20));
  Async<bool> completionToken;
  auto isComplete = false;
  auto expectedValue = 0;
  auto chainReactor = MakeChainReactor(get<0>(initialRange),
    get<0>(continuationRange));
  auto testReactor =
    Do(
      [&] (int value) {
        CPPUNIT_ASSERT(!isComplete);
        if(value != expectedValue) {
          completionToken.GetEval().SetResult(false);
        }
        if(value == 20) {
          completionToken.GetEval().SetResult(true);
          isComplete = true;
        } else {
          ++expectedValue;
        }
      }, chainReactor);
  ReactorMonitor monitor;
  monitor.AddEvent(get<1>(initialRange));
  monitor.AddEvent(get<1>(continuationRange));
  monitor.AddEvent(chainReactor);
  monitor.AddReactor(testReactor);
  monitor.Open();
  auto testResult = completionToken.Get();
  CPPUNIT_ASSERT(testResult);
  CPPUNIT_ASSERT(chainReactor->IsComplete());
}
