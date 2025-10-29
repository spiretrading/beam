#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/WeakQueueWriter.hpp"

using namespace Beam;

TEST_SUITE("WeakQueueWriter") {
  TEST_CASE("push") {
    auto queue = std::make_shared<Queue<int>>();
    auto weak_writer = WeakQueueWriter(queue);
    weak_writer.push(42);
    REQUIRE(queue->pop() == 42);
  }

  TEST_CASE("close") {
    auto queue = std::make_shared<Queue<int>>();
    auto weak_writer = make_weak_queue_writer(queue);
    weak_writer->close(std::runtime_error("closed"));
    REQUIRE_THROWS_AS(queue->pop(), std::runtime_error);
  }

  TEST_CASE("expired") {
    auto queue = std::make_shared<Queue<int>>();
    auto weak_writer = make_weak_queue_writer(queue);
    queue.reset();
    REQUIRE_THROWS_AS(weak_writer->push(1), PipeBrokenException);
  }
}
