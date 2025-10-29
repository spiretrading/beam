#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/ConverterQueueReader.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("ConverterQueueReader") {
  TEST_CASE("push") {
    auto source = std::make_shared<Queue<int>>();
    auto converter = ConverterQueueReader(source, [] (auto value) {
      return std::to_string(value);
    });
    source->push(12);
    REQUIRE(converter.pop() == "12");
    converter.close();
    REQUIRE(source->is_broken());
    REQUIRE_THROWS_AS(converter.pop(), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto source = std::make_shared<Queue<int>>();
    {
      auto converter = convert(source, [] (auto) { return false; });
    }
    REQUIRE(source->is_broken());
  }
}
