#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/ConverterQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("ConverterQueueWriter") {
  TEST_CASE("push") {
    auto destination = std::make_shared<Queue<std::string>>();
    auto converter = MakeConverterQueueWriter<int>(destination,
      [] (auto value) {
        return std::to_string(value);
      });
    converter->Push(54);
    REQUIRE(destination->Pop() == "54");
    converter->Break();
    REQUIRE(destination->IsBroken());
    REQUIRE_THROWS_AS(converter->Push(1), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto destination = std::make_shared<Queue<std::string>>();
    {
      auto converter = MakeConverterQueueWriter<int>(destination,
        [] (auto) {
          return std::string();
        });
    }
    REQUIRE(destination->IsBroken());
  }
}
