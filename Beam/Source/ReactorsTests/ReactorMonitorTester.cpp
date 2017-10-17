#include "Beam/ReactorsTests/ReactorMonitorTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/DoReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void ReactorMonitorTester::TestAddSingleReactorBeforeOpen() {
  ReactorMonitor monitor;
  auto p1 = MakeBasicReactor<int>(Ref(monitor.GetTrigger()));
  bool update = false;
  auto d = Do(
    [&] (int value) {
      CPPUNIT_ASSERT(value == 100);
      update = true;
    }, p1);
  monitor.Add(d);
  monitor.Open();
  p1->Update(100);
  monitor.Close();
  CPPUNIT_ASSERT(update);
}

void ReactorMonitorTester::TestAddSingleReactorAfterOpen() {
  ReactorMonitor monitor;
  auto p1 = MakeBasicReactor<int>(Ref(monitor.GetTrigger()));
  bool update = false;
  auto d = Do(
    [&] (int value) {
      CPPUNIT_ASSERT(value == 100);
      update = true;
    }, p1);
  monitor.Open();
  monitor.Add(d);
  p1->Update(100);
  monitor.Close();
  CPPUNIT_ASSERT(update);
}
