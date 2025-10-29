#include <doctest/doctest.h>
#include "Beam/Queues/TaskQueue.hpp"

using namespace Beam;

TEST_SUITE("TaskQueue") {
  TEST_CASE("slot_after_break") {
    auto queue = TaskQueue();
    queue.close(std::runtime_error("broken"));
    auto received_break = false;
    auto slot = queue.get_slot<int>([] (auto) {}, [&] (const auto&) {
      received_break = true;
    });
    queue.pop()();
    queue.pop()();
    REQUIRE(received_break);
  }

  TEST_CASE("flush") {
    auto queue = TaskQueue();
    auto values = std::vector<int>();
    auto slot = queue.get_slot<int>([&] (auto v) {
      values.push_back(v);
    });
    slot.push(42);
    flush(queue);
    REQUIRE(values.size() == 1);
    REQUIRE(values.front() == 42);
  }

  TEST_CASE("try_pop") {
    auto queue = TaskQueue();
    REQUIRE(!queue.try_pop());
  }

  TEST_CASE("close") {
    auto queue = TaskQueue();
    auto slot = queue.get_slot<int>([] (auto) {});
    queue.close(std::runtime_error("closed"));
    queue.pop()();
    queue.pop()();
    REQUIRE_THROWS_AS(queue.pop(), std::runtime_error);
  }

  TEST_CASE("multiple_slots_receive_pushed_values") {
    auto queue = TaskQueue();
    auto values1 = std::vector<int>();
    auto values2 = std::vector<int>();
    auto slot1 = queue.get_slot<int>([&] (auto v) { values1.push_back(v); });
    auto slot2 = queue.get_slot<int>([&] (auto v) { values2.push_back(v); });
    slot1.push(1);
    slot2.push(2);
    flush(queue);
    REQUIRE(values1.size() == 1);
    REQUIRE(values1.front() == 1);
    REQUIRE(values2.size() == 1);
    REQUIRE(values2.front() == 2);
  }
}
