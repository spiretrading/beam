#include "Beam/ReactorsTests/ReactorMonitorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void ReactorMonitorTester::TestAddSingleReactorBeforeOpen() {
  ReactorMonitor monitor;
  auto queue = std::make_shared<Queue<int>>();
}
