#include <array>
#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/QueuedReader.hpp"

using namespace Beam;

TEST_SUITE("QueuedReader") {
  TEST_CASE("read") {
    auto reader = QueuedReader(BufferReader(from<SharedBuffer>("world")));
    auto buffer = SharedBuffer();
    REQUIRE(reader.read(out(buffer)) == 5);
    REQUIRE(buffer == "world");
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("read_partial") {
    auto reader = QueuedReader(BufferReader(from<SharedBuffer>("abcde")));
    auto part1 = SharedBuffer();
    auto read1 = reader.read(out(part1), 2);
    REQUIRE(read1 == 2);
    REQUIRE(part1 == "ab");
    auto part2 = SharedBuffer();
    auto read2 = reader.read(out(part2), 3);
    REQUIRE(read2 == 3);
    REQUIRE(part2 == "cde");
    REQUIRE_THROWS_AS(reader.read(out(part2), 1), EndOfFileException);
  }
}
