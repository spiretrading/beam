#include <atomic>
#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;

TEST_SUITE("Queue") {
  TEST_CASE("break") {
    auto q = Queue<int>();
    auto exceptionCount = std::atomic_int(0);
    auto r1 = RoutineHandler(Spawn(
      [&] {
        try {
          q.Pop();
        } catch(const PipeBrokenException&) {
          ++exceptionCount;
        }
      }));
    auto r2 = RoutineHandler(Spawn(
      [&] {
        try {
          q.Pop();
        } catch(const PipeBrokenException&) {
          ++exceptionCount;
        }
      }));
    q.Break();
    r1.Wait();
    r2.Wait();
    REQUIRE(exceptionCount == 2);
  }
}
