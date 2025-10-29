#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/StatePublisher.hpp"

using namespace boost;
using namespace Beam;

TEST_SUITE("StatePublisher") {
  TEST_CASE("monitor_push_initial_and_subsequent_updates") {
    auto publisher = StatePublisher(5);
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    auto popped = queue->pop();
    REQUIRE(popped == 5);
    publisher.push(7);
    popped = queue->pop();
    REQUIRE(popped == 7);
    auto snapshot = publisher.get_snapshot();
    REQUIRE(snapshot.has_value());
    REQUIRE(*snapshot == 7);
  }

  TEST_CASE("monitor_with_snapshot_out_receives_current_value") {
    auto publisher = StatePublisher<int>();
    publisher.push(10);
    auto queue = std::make_shared<Queue<int>>();
    auto snapshot = optional<int>();
    publisher.monitor(queue, out(snapshot));
    REQUIRE(snapshot.has_value());
    REQUIRE(*snapshot == 10);
  }

  TEST_CASE("push_rvalue_updates_snapshot_and_notifies") {
    auto publisher = StatePublisher<std::string>();
    auto queue = std::make_shared<Queue<std::string>>();
    publisher.push(std::string("hello"));
    publisher.monitor(queue);
    auto popped = queue->pop();
    REQUIRE(popped == "hello");
    auto snapshot = publisher.get_snapshot();
    REQUIRE(snapshot.has_value());
    REQUIRE(*snapshot == "hello");
  }

  TEST_CASE("close_clears_snapshot_and_closes_queues") {
    auto publisher = StatePublisher<int>();
    auto queue1 = std::make_shared<Queue<int>>();
    publisher.monitor(queue1);
    publisher.push(2);
    REQUIRE(queue1->pop() == 2);
    publisher.close(std::runtime_error("closed"));
    REQUIRE_THROWS_AS(queue1->pop(), std::runtime_error);
    auto queue2 = std::make_shared<Queue<int>>();
    publisher.monitor(queue2);
    REQUIRE_THROWS_AS(queue2->pop(), std::runtime_error);
  }

  TEST_CASE("with_snapshot_invocable_return_types") {
    auto publisher = StatePublisher(3);
    auto value = publisher.with([&] (optional<const int&> s) {
      if(s) {
        return *s + 1;
      }
      return 0;
    });
    REQUIRE(value == 4);
    auto y = 0;
    auto& ref = publisher.with([&] (optional<const int&> s) -> int& {
      return y;
    });
    ref = 9;
    REQUIRE(y == 9);
    auto seen = false;
    publisher.with([&] (optional<const int&> s) {
      if(s) {
        seen = (*s == 3) || (*s == 9);
      }
    });
    REQUIRE(seen);
  }
}
