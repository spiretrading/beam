#include "Beam/ReactorsTests/ReactorMonitorTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/DoReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace Beam::Routines;
using namespace std;

void ReactorMonitorTester::TestAddSingleReactorBeforeOpen() {
  ReactorMonitor monitor;
  auto p1 = MakeBasicReactor<int>();
  bool update = false;
  Async<void> waitToken;
  auto d = Do(
    [&] (int value) {
      CPPUNIT_ASSERT(value == 100);
      update = true;
      waitToken.GetEval().SetResult();
    }, p1);
  monitor.Add(d);
  monitor.Open();
  p1->Update(100);
  waitToken.Get();
  monitor.Close();
  CPPUNIT_ASSERT(update);
}

void ReactorMonitorTester::TestAddSingleReactorAfterOpen() {
  ReactorMonitor monitor;
  auto p1 = MakeBasicReactor<int>();
  bool update = false;
  Async<void> waitToken;
  auto d = Do(
    [&] (int value) {
      CPPUNIT_ASSERT(value == 100);
      update = true;
      waitToken.GetEval().SetResult();
    }, p1);
  monitor.Open();
  monitor.Add(d);
  p1->Update(100);
  waitToken.Get();
  monitor.Close();
  CPPUNIT_ASSERT(update);
}
