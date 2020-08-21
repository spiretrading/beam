#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReaderPublisher.hpp"
#include "Beam/Queues/SequencePublisher.hpp"

using namespace Beam;

TEST_SUITE("SequencePublisher") {
  TEST_CASE("adaptor") {
    auto queue = std::make_shared<Queue<int>>();
    auto publisher = MakeSequencePublisherAdaptor(
      std::make_unique<QueueReaderPublisher<int>>(queue));
    queue->Push(3);
    queue->Push(1);
    queue->Push(4);
    auto monitor = std::make_shared<Queue<int>>();
    publisher->Monitor(monitor);
    REQUIRE(monitor->Pop() == 3);
    REQUIRE(monitor->Pop() == 1);
    REQUIRE(monitor->Pop() == 4);
  }

  TEST_CASE("monitor_snapshot") {
    auto publisher = SequencePublisher<int>();
    publisher.Push(3);
    publisher.Push(1);
    publisher.Push(4);
    auto monitor = std::make_shared<Queue<int>>();
    auto snapshot = boost::optional<std::vector<int>>();
    publisher.Monitor(monitor, Store(snapshot));
    REQUIRE(snapshot.is_initialized());
    REQUIRE(*snapshot == std::vector{3, 1, 4});
    auto getter = publisher.GetSnapshot();
    REQUIRE(getter.is_initialized());
    REQUIRE(*getter == std::vector{3, 1, 4});
  }
}
