#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/ConverterQueueReader.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("ConverterQueueReader") {
  TEST_CASE("push") {
    auto source = std::make_shared<Queue<int>>();
    auto converter = ConverterQueueReader(source,
      [] (int value) {
        return std::to_string(value);
      });
  }
}
