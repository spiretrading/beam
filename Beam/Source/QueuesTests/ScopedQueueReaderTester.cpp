#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"

using namespace Beam;

TEST_SUITE("ScopedQueueReader") {
  TEST_CASE("move_construct") {
    auto queue = std::make_shared<Queue<int>>();
    queue->push(5);
    auto source = ScopedQueueReader(queue);
    auto target = ScopedQueueReader(std::move(source));
    REQUIRE(target.pop() == 5);
    REQUIRE_THROWS_AS(source.pop(), PipeBrokenException);
  }

  TEST_CASE("move_assignment") {
    auto queue = std::make_shared<Queue<int>>();
    queue->push(10);
    auto source = ScopedQueueReader(queue);
    auto target = ScopedQueueReader(std::make_shared<Queue<int>>());
    target = std::move(source);
    REQUIRE(target.pop() == 10);
    REQUIRE_THROWS_AS(source.pop(), PipeBrokenException);
  }

  TEST_CASE("push_and_pop") {
    auto queue = std::make_shared<Queue<int>>();
    queue->push(1);
    queue->push(2);
    auto reader = ScopedQueueReader(queue);
    REQUIRE(reader.pop() == 1);
    REQUIRE(reader.pop() == 2);
  }

  TEST_CASE("try_pop_empty") {
    auto queue = std::make_shared<Queue<int>>();
    auto reader = ScopedQueueReader(queue);
    auto result = reader.try_pop();
    REQUIRE(!result);
  }

  TEST_CASE("close_forwarded") {
    auto queue = std::make_shared<Queue<int>>();
    queue->push(42);
    auto reader = ScopedQueueReader<int>(queue);
    reader.close(std::runtime_error("broken"));
    REQUIRE(reader.pop() == 42);
    REQUIRE_THROWS_AS(reader.pop(), std::runtime_error);
  }
}
