#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/ScopedQueueGroup.hpp"

using namespace Beam;

TEST_SUITE("ScopedQueueGroup") {
  TEST_CASE("add_and_close_default") {
    auto group = ScopedQueueGroup();
    auto queue1 = std::make_shared<Queue<int>>();
    auto queue2 = std::make_shared<Queue<double>>();
    group.add(queue1);
    group.add(queue2);
    REQUIRE(!queue1->is_broken());
    REQUIRE(!queue2->is_broken());
    group.close();
    REQUIRE(queue1->is_broken());
    REQUIRE(queue2->is_broken());
    REQUIRE_THROWS_AS(queue1->pop(), PipeBrokenException);
    REQUIRE_THROWS_AS(queue2->pop(), PipeBrokenException);
  }

  TEST_CASE("destructor_closes_queues") {
    auto queue1 = std::make_shared<Queue<int>>();
    auto queue2 = std::make_shared<Queue<double>>();
    {
      auto group = ScopedQueueGroup();
      group.add(queue1);
      group.add(queue2);
      REQUIRE(!queue1->is_broken());
      REQUIRE(!queue2->is_broken());
    }
    REQUIRE(queue1->is_broken());
    REQUIRE(queue2->is_broken());
    REQUIRE_THROWS_AS(queue1->pop(), PipeBrokenException);
    REQUIRE_THROWS_AS(queue2->pop(), PipeBrokenException);
  }

  TEST_CASE("add_returns_queue") {
    auto group = ScopedQueueGroup();
    auto queue = std::make_shared<Queue<int>>();
    auto returned = group.add(queue);
    REQUIRE(returned == queue);
  }

  TEST_CASE("close_with_exception_ptr") {
    auto group = ScopedQueueGroup();
    auto queue1 = std::make_shared<Queue<int>>();
    auto queue2 = std::make_shared<Queue<std::string>>();
    group.add(queue1);
    group.add(queue2);
    auto exception = std::make_exception_ptr(std::runtime_error("test"));
    group.close(exception);
    REQUIRE(queue1->is_broken());
    REQUIRE(queue2->is_broken());
    REQUIRE_THROWS_AS(queue1->pop(), std::runtime_error);
    REQUIRE_THROWS_AS(queue2->pop(), std::runtime_error);
  }

  TEST_CASE("close_with_exception_object") {
    auto group = ScopedQueueGroup();
    auto queue1 = std::make_shared<Queue<int>>();
    auto queue2 = std::make_shared<Queue<int>>();
    group.add(queue1);
    group.add(queue2);
    group.close(std::logic_error("custom error"));
    REQUIRE(queue1->is_broken());
    REQUIRE(queue2->is_broken());
    REQUIRE_THROWS_AS(queue1->pop(), std::logic_error);
    REQUIRE_THROWS_AS(queue2->pop(), std::logic_error);
  }

  TEST_CASE("close_empty_group") {
    auto group = ScopedQueueGroup();
    group.close();
  }

  TEST_CASE("add_after_close") {
    auto group = ScopedQueueGroup();
    auto queue1 = std::make_shared<Queue<int>>();
    group.add(queue1);
    group.close();
    REQUIRE(queue1->is_broken());
    auto queue2 = std::make_shared<Queue<int>>();
    group.add(queue2);
    REQUIRE(!queue2->is_broken());
  }

  TEST_CASE("multiple_closes") {
    auto group = ScopedQueueGroup();
    auto queue1 = std::make_shared<Queue<int>>();
    auto queue2 = std::make_shared<Queue<int>>();
    group.add(queue1);
    group.add(queue2);
    group.close(std::runtime_error("first"));
    REQUIRE(queue1->is_broken());
    REQUIRE(queue2->is_broken());
    auto queue3 = std::make_shared<Queue<int>>();
    group.add(queue3);
    group.close(std::logic_error("second"));
    REQUIRE(queue3->is_broken());
    REQUIRE_THROWS_AS(queue3->pop(), std::logic_error);
  }
}
