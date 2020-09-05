#include <atomic>
#include <doctest/doctest.h>
#include "Beam/Queues/StateQueue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;

TEST_SUITE("StateQueue") {
  TEST_CASE("break") {
    auto q = StateQueue<int>();
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

  TEST_CASE("peek") {
    auto q = StateQueue<int>();
    q.Push(123);
    q.Push(456);
    REQUIRE(q.Peek() == 456);
    REQUIRE(q.Peek() == 456);
    REQUIRE(q.Pop() == 456);
    q.Break();
    REQUIRE_THROWS_AS(q.Peek(), PipeBrokenException);
  }
}
