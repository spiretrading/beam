#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/FilteredQueueReader.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("FilteredQueueReader") {
  TEST_CASE("push") {
    auto source = std::make_shared<Queue<int>>();
    auto filter = FilteredQueueReader(source,
      [] (auto value) {
        return value % 2 == 0;
      });
    source->Push(1);
    source->Push(2);
    source->Push(3);
    source->Push(4);
    source->Push(5);
    source->Push(6);
    source->Break();
    REQUIRE(filter.Pop() == 2);
    REQUIRE(filter.Pop() == 4);
    REQUIRE(filter.Pop() == 6);
    REQUIRE_THROWS_AS(filter.Pop(), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto source = std::make_shared<Queue<int>>();
    {
      auto filter = MakeFilteredQueueReader(source,
        [] (auto) {
          return false;
        });
    }
    REQUIRE(source->IsBroken());
  }
}
