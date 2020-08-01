#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;

TEST_SUITE("Queue") {
  TEST_CASE("break") {
    auto q = Queue<int>();
    auto r1 = RoutineHandler(Spawn(
      [&] {
        q.Pop();
      }));
    auto r2 = RoutineHandler(Spawn(
      [&] {
        q.Pop();
      }));
    q.Break();
    r1.Wait();
    r2.Wait();
  }
}
