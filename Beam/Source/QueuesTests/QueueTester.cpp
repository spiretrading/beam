#include <stdexcept>
#include <thread>
#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("Queue") {
  TEST_CASE("push_and_pop") {
    auto queue = Queue<int>();
    queue.push(1);
    queue.push(2);
    auto first = queue.pop();
    auto second = queue.pop();
    REQUIRE(first == 1);
    REQUIRE(second == 2);
  }

  TEST_CASE("try_pop_empty") {
    auto queue = Queue<int>();
    auto result = queue.try_pop();
    REQUIRE(!result);
  }

  TEST_CASE("close_with_exception") {
    auto queue = Queue<int>();
    queue.close(std::runtime_error("broken"));
    REQUIRE_THROWS_AS(queue.pop(), std::runtime_error);
  }

  TEST_CASE("push_after_close") {
    auto queue = Queue<int>();
    queue.close(std::runtime_error("broken"));
    REQUIRE_THROWS_AS(queue.push(1), std::runtime_error);
  }

  TEST_CASE("pop_consumes_items") {
    auto queue = Queue<int>();
    queue.push(42);
    queue.close(std::runtime_error("broken"));
    auto value = queue.pop();
    REQUIRE(value == 42);
    REQUIRE_THROWS_AS(queue.pop(), std::runtime_error);
  }

  TEST_CASE("concurrent_push_and_pop") {
    auto count = 5;
    auto queue = Queue<int>();
    auto values = std::vector<int>();
    values.reserve(count);
    auto producer = std::thread([&] () {
      for(auto i = 0; i < count; ++i) {
        queue.push(i);
      }
      queue.close(std::runtime_error("done"));
    });
    for(auto i = 0; i < count; ++i) {
      values.push_back(queue.pop());
    }
    producer.join();
    REQUIRE(values.size() == count);
    for(auto i = 0; i < count; ++i) {
      REQUIRE(values[i] == i);
    }
  }

  TEST_CASE("is_broken_reflects_state") {
    auto queue = Queue<int>();
    REQUIRE(!queue.is_broken());
    queue.push(1);
    queue.close(std::runtime_error("broken"));
    REQUIRE(!queue.is_broken());
    auto value = queue.pop();
    REQUIRE(value == 1);
    REQUIRE(queue.is_broken());
  }
}
