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
}
