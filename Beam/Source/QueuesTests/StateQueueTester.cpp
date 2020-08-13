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
