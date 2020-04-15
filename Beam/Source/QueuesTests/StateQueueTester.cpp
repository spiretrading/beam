#include <doctest/doctest.h>
#include "Beam/Queues/StateQueue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;

TEST_SUITE("StateQueue") {
  TEST_CASE("break") {
    auto q = StateQueue<int>();
    auto r1 = RoutineHandler(Spawn(
      [&] {
        q.Top();
      }));
    auto r2 = RoutineHandler(Spawn(
      [&] {
        q.Top();
      }));
    q.Break();
    r1.Wait();
    r2.Wait();
  }
}
