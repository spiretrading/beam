#include <doctest/doctest.h>
#include "Beam/Queues/AliasQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("AliasQueueWriter") {
  TEST_CASE("immediate_break") {
    auto source = std::make_shared<Queue<int>>();
    auto alias = std::make_shared<int>(123);
    auto queue = MakeAliasQueueWriter(source, alias);
    alias = nullptr;
    REQUIRE(!source->IsBroken());
    REQUIRE_THROWS_AS(queue->Push(1), PipeBrokenException);
    REQUIRE(source->IsBroken());
  }

  TEST_CASE("break") {
    auto source = std::make_shared<Queue<int>>();
    auto alias = std::make_shared<int>(123);
    auto queue = MakeAliasQueueWriter(source, alias);
    queue->Push(123);
    queue->Push(456);
    alias = nullptr;
    REQUIRE_THROWS_AS(queue->Push(1), PipeBrokenException);
    REQUIRE(source->Pop() == 123);
    REQUIRE(source->Pop() == 456);
    REQUIRE_THROWS_AS(source->Pop(), PipeBrokenException);
  }
}
