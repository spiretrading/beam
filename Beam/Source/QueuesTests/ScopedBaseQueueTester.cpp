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
    REQUIRE(q.is_broken());
  }

  TEST_CASE("move") {
    auto q = Queue<int>();
    {
      auto s1 = ScopedBaseQueue(&q);
      auto s2 = std::move(s1);
      REQUIRE(!q.is_broken());
    }
    REQUIRE(q.is_broken());
  }

  TEST_CASE("assign") {
    auto q1 = std::make_shared<Queue<int>>();
    auto q2 = std::make_shared<Queue<int>>();
    {
      auto s1 = ScopedBaseQueue(q1);
      auto s2 = ScopedBaseQueue(q2);
      std::swap(s1, s2);
      REQUIRE(!q1->is_broken());
      REQUIRE(!q2->is_broken());
      s1 = std::move(s2);
      REQUIRE(!q1->is_broken());
      REQUIRE(q2->is_broken());
    }
    REQUIRE(q1->is_broken());
  }
}
