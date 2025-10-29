#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"

using namespace Beam;

TEST_SUITE("QueueWriterPublisher") {
  TEST_CASE("monitor_and_push_single_queue") {
    auto publisher = QueueWriterPublisher<int>();
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    REQUIRE(publisher.get_size() == 1);
    publisher.push(42);
    auto popped = queue->pop();
    REQUIRE(popped == 42);
  }

  TEST_CASE("monitor_multiple_queues_and_push_and_remove_throwing") {
    auto publisher = QueueWriterPublisher<int>();
    auto queue1 = std::make_shared<Queue<int>>();
    auto queue2 = std::make_shared<Queue<int>>();
    publisher.monitor(queue1);
    publisher.monitor(queue2);
    REQUIRE(publisher.get_size() == 2);
    publisher.push(1);
    REQUIRE(queue1->pop() == 1);
    REQUIRE(queue2->pop() == 1);
    queue1->close(std::runtime_error("closed"));
    publisher.push(2);
    REQUIRE_THROWS_AS(queue1->pop(), std::runtime_error);
    REQUIRE(queue2->pop() == 2);
    REQUIRE(publisher.get_size() == 1);
    publisher.push(3);
    REQUIRE(queue2->pop() == 3);
  }

  TEST_CASE("close_and_monitor_after_close") {
    auto publisher = QueueWriterPublisher<int>();
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    REQUIRE(publisher.get_size() == 1);
    publisher.close(std::runtime_error("closed error"));
    REQUIRE_THROWS_AS(queue->pop(), std::runtime_error);
    REQUIRE(publisher.get_size() == 0);
    auto queue2 = std::make_shared<Queue<int>>();
    publisher.monitor(queue2);
    REQUIRE_THROWS_AS(queue2->pop(), std::runtime_error);
    REQUIRE(publisher.get_size() == 0);
  }

  TEST_CASE("with_synchronization_and_return_types") {
    auto publisher = QueueWriterPublisher<int>();
    auto x = 0;
    publisher.with([&] {
      x = 5;
    });
    REQUIRE(x == 5);
    auto value = publisher.with([&] {
      return publisher.get_size();
    });
    REQUIRE(value == 0);
    auto y = 0;
    auto& ref = publisher.with([&]() -> int& {
      return y;
    });
    ref = 7;
    REQUIRE(y == 7);
  }

  TEST_CASE("push_rvalue_with_string") {
    auto publisher = QueueWriterPublisher<std::string>();
    auto queue = std::make_shared<Queue<std::string>>();
    publisher.monitor(queue);
    auto s = std::string("hello");
    publisher.push(std::move(s));
    auto popped = queue->pop();
    REQUIRE(popped == "hello");
  }
}
