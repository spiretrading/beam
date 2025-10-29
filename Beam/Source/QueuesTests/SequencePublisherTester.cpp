#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReaderPublisher.hpp"
#include "Beam/Queues/SequencePublisher.hpp"

using namespace boost;
using namespace Beam;

TEST_SUITE("SequencePublisher") {
  TEST_CASE("adaptor") {
    auto queue = std::make_shared<Queue<int>>();
    auto reader = QueueReaderPublisher(queue);
    auto publisher = make_sequence_publisher_adaptor(&reader);
    queue->push(3);
    queue->push(1);
    queue->push(4);
    auto monitor = std::make_shared<Queue<int>>();
    publisher->monitor(monitor);
    REQUIRE(monitor->pop() == 3);
    REQUIRE(monitor->pop() == 1);
    REQUIRE(monitor->pop() == 4);
  }

  TEST_CASE("monitor_snapshot") {
    auto publisher = SequencePublisher<int>();
    publisher.push(3);
    publisher.push(1);
    publisher.push(4);
    auto monitor = std::make_shared<Queue<int>>();
    auto snapshot = optional<std::vector<int>>();
    publisher.monitor(monitor, out(snapshot));
    REQUIRE(snapshot.has_value());
    REQUIRE(*snapshot == std::vector{3, 1, 4});
    auto getter = publisher.get_snapshot();
    REQUIRE(getter.has_value());
    REQUIRE(*getter == std::vector{3, 1, 4});
  }
}
