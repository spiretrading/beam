#include <doctest/doctest.h>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("MultiQueueWriter") {
  TEST_CASE("break_immediately") {
    auto reader = MultiQueueWriter<int>();
    reader.close();
    REQUIRE_THROWS_AS(reader.push(1), PipeBrokenException);
    REQUIRE_THROWS_AS(reader.pop(), PipeBrokenException);
    REQUIRE(!reader.try_pop());
  }

  TEST_CASE("single_writer") {
    auto reader = MultiQueueWriter<int>();
    auto writer = reader.get_writer();
    writer.push(123);
    REQUIRE(reader.pop() == 123);
    writer.close();
    REQUIRE_THROWS_AS(writer.push(1), PipeBrokenException);
    writer = reader.get_writer();
    writer.push(321);
    REQUIRE(reader.pop() == 321);
    reader.close();
    REQUIRE_THROWS_AS(reader.push(1), PipeBrokenException);
    REQUIRE_THROWS_AS(reader.pop(), PipeBrokenException);
    REQUIRE(!reader.try_pop());
  }
}
