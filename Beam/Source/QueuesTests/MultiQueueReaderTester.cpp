#include <doctest/doctest.h>
#include "Beam/Queues/MultiQueueReader.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("MultiQueueReader") {
  TEST_CASE("break_immediately") {
    auto reader = MultiQueueReader<int>();
    reader.Break();
    REQUIRE_THROWS_AS(reader.Push(1), PipeBrokenException);
    REQUIRE_THROWS_AS(reader.Pop(), PipeBrokenException);
    REQUIRE(!reader.TryPop());
  }

  TEST_CASE("single_writer") {
    auto reader = MultiQueueReader<int>();
    auto writer = reader.GetWriter();
    writer.Push(123);
    REQUIRE(reader.Pop() == 123);
    writer.Break();
    REQUIRE_THROWS_AS(writer.Push(1), PipeBrokenException);
    writer = reader.GetWriter();
    writer.Push(321);
    REQUIRE(reader.Pop() == 321);
    reader.Break();
    REQUIRE_THROWS_AS(reader.Push(1), PipeBrokenException);
    REQUIRE_THROWS_AS(reader.Pop(), PipeBrokenException);
    REQUIRE(!reader.TryPop());
  }
}
