#include <doctest/doctest.h>
#include "Beam/Queues/TaskQueue.hpp"

using namespace Beam;

TEST_SUITE("TaskQueue") {
  TEST_CASE("slot_after_break") {
    auto queue = TaskQueue();
    queue.Break();
    auto receivedBreak = false;
    auto slot = queue.GetSlot<int>([] (auto) {},
      [&] (const auto& e) {
        receivedBreak = true;
      });
    slot.Break();
    REQUIRE(receivedBreak);
  }
}
