#include <doctest/doctest.h>
#include "Beam/Queues/RoutineTaskQueue.hpp"

using namespace Beam;

TEST_SUITE("RoutineTaskQueue") {
  TEST_CASE("push_slot_executes") {
    auto queue = RoutineTaskQueue();
    auto values = std::vector<int>();
    auto slot = queue.get_slot<int>([&] (auto v) {
      values.push_back(v);
    });
    slot.push(42);
    queue.close();
    queue.wait();
    REQUIRE(values.size() == 1);
    REQUIRE(values.front() == 42);
  }

  TEST_CASE("multiple_slots_receive_values") {
    auto queue = RoutineTaskQueue();
    auto values1 = std::vector<int>();
    auto values2 = std::vector<int>();
    auto slot1 = queue.get_slot<int>([&] (auto v) {
      values1.push_back(v);
    });
    auto slot2 = queue.get_slot<int>([&] (auto v) {
      values2.push_back(v);
    });
    slot1.push(1);
    slot2.push(2);
    queue.close();
    queue.wait();
    REQUIRE(values1.size() == 1);
    REQUIRE(values1.front() == 1);
    REQUIRE(values2.size() == 1);
    REQUIRE(values2.front() == 2);
  }

  TEST_CASE("push_function_executes") {
    auto queue = RoutineTaskQueue();
    auto executed = false;
    queue.push([&] {
      executed = true;
    });
    queue.close();
    queue.wait();
    REQUIRE(executed);
  }

  TEST_CASE("close_wait_noop") {
    auto queue = RoutineTaskQueue();
    REQUIRE_NOTHROW(queue.close());
    REQUIRE_NOTHROW(queue.wait());
  }
}
