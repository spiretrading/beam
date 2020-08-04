#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/ConverterQueueReader.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("ConverterQueueReader") {
  TEST_CASE("push") {
    auto source = std::make_shared<Queue<int>>();
    auto converter = ConverterQueueReader(source,
      [] (auto value) {
        return std::to_string(value);
      });
    source->Push(12);
    REQUIRE(converter.Pop() == "12");
    converter.Break();
    REQUIRE(source->IsBroken());
    REQUIRE_THROWS_AS(converter.Pop(), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto source = std::make_shared<Queue<int>>();
    {
      auto converter = MakeConverterQueueReader(source,
        [] (auto) { return false; });
    }
    REQUIRE(source->IsBroken());
  }
}
