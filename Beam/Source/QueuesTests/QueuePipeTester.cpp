#include <doctest/doctest.h>
#include "Beam/Queues/QueuePipe.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("QueuePipe") {
  TEST_CASE("break") {
    auto source = std::make_shared<Queue<int>>();
    auto destination = std::make_shared<Queue<double>>();
    {
      auto pipe = QueuePipe(source, destination);
    }
    REQUIRE(source->is_broken());
    REQUIRE(destination->is_broken());
  }

  TEST_CASE("convert") {
    auto source = std::make_shared<Queue<int>>();
    auto destination = std::make_shared<Queue<double>>();
    auto pipe = QueuePipe(source, destination);
    source->push(123);
    REQUIRE(destination->pop() == 123.0);
  }
}
