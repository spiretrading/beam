#include <doctest/doctest.h>
#include "Beam/Queues/QueueReaderPublisher.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("QueueReaderPublisher") {
  TEST_CASE("break") {
    auto queue = std::make_shared<Queue<int>>();
    {
      auto publisher = QueueReaderPublisher(queue);
    }
    REQUIRE(queue->IsBroken());
  }

  TEST_CASE("publish") {
    auto source = std::make_shared<Queue<int>>();
    auto publisher = QueueReaderPublisher(source);
    auto destinationA = std::make_shared<Queue<int>>();
    auto destinationB = std::make_shared<Queue<int>>();
    publisher.Monitor(destinationA);
    publisher.Monitor(destinationB);
    source->Push(123);
    REQUIRE(destinationA->Pop() == 123);
    REQUIRE(destinationB->Pop() == 123);
  }
}
