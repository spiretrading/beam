#include <array>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("Reader") {
  TEST_CASE("construct_from_reader_value") {
    auto shared = from<SharedBuffer>("copyctor");
    auto base = BufferReader(shared);
    auto reader = Reader(base);
    auto destination = SharedBuffer();
    auto read_count = reader.read(out(destination));
    REQUIRE(read_count == shared.get_size());
    REQUIRE(destination == shared);
  }

  TEST_CASE("read") {
    auto shared = from<SharedBuffer>("payload");
    auto reader =
      Reader(std::in_place_type_t<BufferReader<SharedBuffer>>(), shared);
    auto destination = SharedBuffer();
    auto read_count = reader.read(out(destination));
    REQUIRE(read_count == shared.get_size());
    REQUIRE(destination == shared);
  }

  TEST_CASE("read__size") {
    auto shared = from<SharedBuffer>("examples");
    auto reader =
      Reader(std::in_place_type_t<BufferReader<SharedBuffer>>(), shared);
    auto destination = SharedBuffer();
    auto read_count = reader.read(out(destination), 4);
    REQUIRE(read_count == 4);
    REQUIRE(destination == "exam");
    read_count = reader.read(out(destination));
    REQUIRE(read_count == shared.get_size() - 4);
    REQUIRE(destination == "examples");
  }

  TEST_CASE("read_exact") {
    auto shared = from<SharedBuffer>("bufferexact");
    auto reader =
      Reader(std::in_place_type_t<BufferReader<SharedBuffer>>(), shared);
    auto destination = SharedBuffer();
    read_exact(reader, out(destination), shared.get_size());
    REQUIRE(destination == shared);
  }

  TEST_CASE("poll") {
    auto reader = Reader(std::in_place_type_t<BufferReader<SharedBuffer>>(),
      from<SharedBuffer>("xy"));
    REQUIRE(reader.poll());
    auto data = SharedBuffer();
    reader.read(out(data));
    REQUIRE(!reader.poll());
  }

  TEST_CASE("read_past_eof") {
    auto shared = from<SharedBuffer>("eof");
    auto reader =
      Reader(std::in_place_type_t<BufferReader<SharedBuffer>>(), shared);
    auto total = shared.get_size();
    auto buffer = SharedBuffer();
    auto read_count = reader.read(out(buffer));
    REQUIRE(read_count == total);
    REQUIRE_THROWS_AS(reader.read(out(buffer), 1), EndOfFileException);
    auto destination = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(destination)), EndOfFileException);
  }
}
