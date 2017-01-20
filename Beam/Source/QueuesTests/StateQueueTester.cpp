#include "Beam/QueuesTests/StateQueueTester.hpp"
#include "Beam/Queues/StateQueue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Tests;
using namespace std;

void StateQueueTester::TestBreak() {
  StateQueue<int> q;
  RoutineHandler r1 = Spawn(
    [&] {
      q.Top();
    });
  RoutineHandler r2 = Spawn(
    [&] {
      q.Top();
    });
  q.Break();
  r1.Wait();
  r2.Wait();
}
