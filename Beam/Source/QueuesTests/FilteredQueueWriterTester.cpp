#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/FilteredQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("FilteredQueueWriter") {
  TEST_CASE("push") {
    auto destination = std::make_shared<Queue<std::string>>();
    auto filter = FilteredQueueWriter(destination,
      [] (const auto& value) {
        return !value.empty() && value[0] == 'g';
      });
    filter.Push("hello");
    filter.Push("goodbye");
    filter.Push("opinion");
    filter.Push("global");
    filter.Push("genesis");
    filter.Break();
    REQUIRE(destination->Pop() == "goodbye");
    REQUIRE(destination->Pop() == "global");
    REQUIRE(destination->Pop() == "genesis");
    REQUIRE(destination->IsBroken());
    REQUIRE_THROWS_AS(filter.Push("alpha"), PipeBrokenException);
    REQUIRE_THROWS_AS(filter.Push("gamma"), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto destination = std::make_shared<Queue<std::string>>();
    {
      auto filter = MakeFilteredQueueWriter(destination,
        [] (auto) {
          return false;
        });
    }
    REQUIRE(destination->IsBroken());
  }
}
