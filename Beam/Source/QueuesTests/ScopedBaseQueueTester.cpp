#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/ScopedBaseQueue.hpp"

using namespace Beam;

TEST_SUITE("ScopedBaseQueue") {
  TEST_CASE("break") {
    auto q = Queue<int>();
    {
      auto s = ScopedBaseQueue(&q);
    }
    REQUIRE(q.IsBroken());
  }

  TEST_CASE("move") {
    auto q = Queue<int>();
    {
      auto s1 = ScopedBaseQueue(&q);
      auto s2 = std::move(s1);
      REQUIRE(!q.IsBroken());
    }
    REQUIRE(q.IsBroken());
  }

  TEST_CASE("assign") {
    auto q1 = std::make_shared<Queue<int>>();
    auto q2 = std::make_shared<Queue<int>>();
    {
      auto s1 = ScopedBaseQueue(q1);
      auto s2 = ScopedBaseQueue(q2);
      std::swap(s1, s2);
      REQUIRE(!q1->IsBroken());
      REQUIRE(!q2->IsBroken());
      s1 = std::move(s2);
      REQUIRE(q2->IsBroken());
      REQUIRE(!q1->IsBroken());
    }
    REQUIRE(q1->IsBroken());
  }
}
