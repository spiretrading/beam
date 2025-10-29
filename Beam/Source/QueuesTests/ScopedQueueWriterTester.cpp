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
    REQUIRE(q.is_broken());
  }

  TEST_CASE("move") {
    auto q = Queue<int>();
    {
      auto s1 = ScopedQueueWriter(&q);
      auto s2 = std::move(s1);
      REQUIRE(!q.is_broken());
    }
    REQUIRE(q.is_broken());
  }

  TEST_CASE("move_convert") {
    auto q = std::make_shared<Queue<int>>();
    auto s1 = ScopedQueueWriter(q);
    auto s2 =
      ScopedQueueWriter<int, std::shared_ptr<QueueWriter<int>>>(std::move(s1));
    REQUIRE(!q->is_broken());
    s2.push(123);
    REQUIRE(q->pop() == 123);
  }

  TEST_CASE("assign") {
    auto q1 = std::make_shared<Queue<int>>();
    auto q2 = std::make_shared<Queue<int>>();
    {
      auto s1 = ScopedQueueWriter(q1);
      auto s2 = ScopedQueueWriter(q2);
      std::swap(s1, s2);
      REQUIRE(!q1->is_broken());
      REQUIRE(!q2->is_broken());
      s1 = std::move(s2);
      REQUIRE(q2->is_broken());
      REQUIRE(!q1->is_broken());
    }
    REQUIRE(q1->is_broken());
  }
}
