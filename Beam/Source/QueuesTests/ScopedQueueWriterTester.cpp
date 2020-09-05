#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

using namespace Beam;

TEST_SUITE("ScopedQueueWriter") {
  TEST_CASE("break") {
    auto q = Queue<int>();
    {
      auto s = ScopedQueueWriter(&q);
    }
    REQUIRE(q.IsBroken());
  }

  TEST_CASE("move") {
    auto q = Queue<int>();
    {
      auto s1 = ScopedQueueWriter(&q);
      auto s2 = std::move(s1);
      REQUIRE(!q.IsBroken());
    }
    REQUIRE(q.IsBroken());
  }

  TEST_CASE("move_convert") {
    auto q = std::make_shared<Queue<int>>();
    auto s1 = ScopedQueueWriter(q);
    auto s2 = ScopedQueueWriter<int, std::shared_ptr<QueueWriter<int>>>(
      std::move(s1));
    REQUIRE(!q->IsBroken());
    s2.Push(123);
    REQUIRE(q->Pop() == 123);
  }

  TEST_CASE("assign") {
    auto q1 = std::make_shared<Queue<int>>();
    auto q2 = std::make_shared<Queue<int>>();
    {
      auto s1 = ScopedQueueWriter(q1);
      auto s2 = ScopedQueueWriter(q2);
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
