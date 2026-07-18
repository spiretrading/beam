module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>
#include "Beam/Queues/Queue.hpp"

module Beam;

using namespace Beam;

TEST_SUITE("ConverterQueueWriter") {
  TEST_CASE("push") {
    auto destination = std::make_shared<Queue<std::string>>();
    auto converter = convert<int>(destination, [] (auto value) {
      return std::to_string(value);
    });
    converter->push(54);
    REQUIRE(destination->pop() == "54");
    converter->close();
    REQUIRE(destination->is_broken());
    REQUIRE_THROWS_AS(converter->push(1), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto destination = std::make_shared<Queue<std::string>>();
    {
      auto converter = convert<int>(destination, [] (auto) {
        return std::string();
      });
    }
    REQUIRE(destination->is_broken());
  }
}
