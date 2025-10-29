#include <atomic>
#include <doctest/doctest.h>
#include "Beam/Queues/StateQueue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;

TEST_SUITE("StateQueue") {
  TEST_CASE("break") {
    auto q = StateQueue<int>();
    auto exception_count = std::atomic_int(0);
    auto r1 = RoutineHandler(spawn([&] {
      try {
        q.pop();
      } catch(const PipeBrokenException&) {
        ++exception_count;
      }
    }));
    auto r2 = RoutineHandler(spawn([&] {
      try {
        q.pop();
      } catch(const PipeBrokenException&) {
        ++exception_count;
      }
    }));
    q.close();
    r1.wait();
    r2.wait();
    REQUIRE(exception_count == 2);
  }

  TEST_CASE("peek") {
    auto q = StateQueue<int>();
    q.push(123);
    q.push(456);
    REQUIRE(q.peek() == 456);
    REQUIRE(q.peek() == 456);
    REQUIRE(q.pop() == 456);
    q.close();
    REQUIRE_THROWS_AS(q.peek(), PipeBrokenException);
  }
}
