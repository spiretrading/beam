#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/FilteredQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("FilteredQueueWriter") {
  TEST_CASE("push") {
    auto destination = std::make_shared<Queue<std::string>>();
    auto filter = FilteredQueueWriter(destination, [] (const auto& value) {
      return !value.empty() && value[0] == 'g';
    });
    filter.push("hello");
    filter.push("goodbye");
    filter.push("opinion");
    filter.push("global");
    filter.push("genesis");
    filter.close();
    REQUIRE(destination->pop() == "goodbye");
    REQUIRE(destination->pop() == "global");
    REQUIRE(destination->pop() == "genesis");
    REQUIRE(destination->is_broken());
    REQUIRE_THROWS_AS(filter.push("alpha"), PipeBrokenException);
    REQUIRE_THROWS_AS(filter.push("gamma"), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto destination = std::make_shared<Queue<std::string>>();
    {
      auto f = filter(destination, [] (auto) {
        return false;
      });
    }
    REQUIRE(destination->is_broken());
  }
}
