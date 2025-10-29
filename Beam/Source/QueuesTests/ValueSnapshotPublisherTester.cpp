#include <doctest/doctest.h>
#include "Beam/Queues/ValueSnapshotPublisher.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace boost;
using namespace Beam;

TEST_SUITE("ValueSnapshotPublisher") {
  TEST_CASE("initialization") {
    auto initialize_called = false;
    auto publisher = ValueSnapshotPublisher<int, int>(
      [&] (const auto& snapshot, auto& monitor) {
        initialize_called = true;
        monitor.push(snapshot + 1);
      }, [&] (auto& snapshot, const auto& value) {
        snapshot = value;
      }, 10);
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    REQUIRE(initialize_called);
    REQUIRE(queue->pop() == 11);
  }

  TEST_CASE("monitor") {
    auto publisher = ValueSnapshotPublisher<int, int>(
      [] (const auto&, auto&) {},
      [] (auto& snapshot, const auto& value) {
        snapshot = value;
      }, 5);
    auto queue = std::make_shared<Queue<int>>();
    auto snapshot = optional<int>();
    publisher.monitor(ScopedQueueWriter(queue), out(snapshot));
    REQUIRE(snapshot.has_value());
    REQUIRE(*snapshot == 5);
  }

  TEST_CASE("push") {
    auto publisher = ValueSnapshotPublisher<int, int>(
      [] (const auto&, auto&) {},
      [] (auto& snapshot, const auto& value) {
        snapshot = value + 100;
      }, 1);
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(ScopedQueueWriter(queue));
    publisher.push(7);
    REQUIRE(queue->pop() == 7);
    auto snapshot = publisher.get_snapshot();
    REQUIRE(snapshot.has_value());
    REQUIRE(*snapshot == 107);
  }

  TEST_CASE("filtered_update") {
    auto publisher = ValueSnapshotPublisher<int, int>(true,
      [] (const auto&, auto&) {},
      [] (auto& snapshot, const auto& value) {
        snapshot = value;
        return (value % 2) == 0;
      }, 0);
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    publisher.push(3);
    auto popped = queue->try_pop();
    REQUIRE(!popped.has_value());
    auto snapshot1 = publisher.get_snapshot();
    REQUIRE(snapshot1.has_value());
    REQUIRE(*snapshot1 == 3);
    publisher.push(4);
    REQUIRE(queue->pop() == 4);
    auto snapshot2 = publisher.get_snapshot();
    REQUIRE(snapshot2.has_value());
    REQUIRE(*snapshot2 == 4);
  }

  TEST_CASE("close") {
    auto publisher = ValueSnapshotPublisher<int, int>(
      [] (const auto&, auto&) {},
      [] (auto& snapshot, const auto& value) {
        snapshot = value;
      }, 0);
    auto queue1 = std::make_shared<Queue<int>>();
    publisher.monitor(ScopedQueueWriter(queue1));
    publisher.close(std::runtime_error("closed"));
    REQUIRE_THROWS_AS(queue1->pop(), std::runtime_error);
    auto queue2 = std::make_shared<Queue<int>>();
    publisher.monitor(ScopedQueueWriter(queue2));
    REQUIRE_THROWS_AS(queue2->pop(), std::runtime_error);
  }
}
