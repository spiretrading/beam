#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/FilteredQueueReader.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("FilteredQueueReader") {
  TEST_CASE("push") {
    auto source = std::make_shared<Queue<int>>();
    auto filter = FilteredQueueReader(source, [] (auto value) {
      return value % 2 == 0;
    });
    source->push(1);
    source->push(2);
    source->push(3);
    source->push(4);
    source->push(5);
    source->push(6);
    source->close();
    REQUIRE(filter.pop() == 2);
    REQUIRE(filter.pop() == 4);
    REQUIRE(filter.pop() == 6);
    REQUIRE_THROWS_AS(filter.pop(), PipeBrokenException);
  }

  TEST_CASE("break_on_destroy") {
    auto source = std::make_shared<Queue<int>>();
    {
      auto f = filter(source, [] (auto) {
        return false;
      });
    }
    REQUIRE(source->is_broken());
  }
}
