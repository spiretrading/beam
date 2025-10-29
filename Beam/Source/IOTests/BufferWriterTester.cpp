#include <doctest/doctest.h>
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("BufferWriter") {
  TEST_CASE("write") {
    auto buffer = SharedBuffer();
    auto writer = BufferWriter(Ref(buffer));
    auto source = from<SharedBuffer>("abc");
    writer.write(source);
    REQUIRE(buffer == source);
  }

  TEST_CASE("multiple_writes") {
    auto buffer = SharedBuffer();
    auto writer = BufferWriter(Ref(buffer));
    auto first = from<SharedBuffer>("ab");
    auto second = from<SharedBuffer>("cd");
    writer.write(first);
    writer.write(second);
    REQUIRE(buffer == "abcd");
  }

  TEST_CASE("preserves_existing_data_and_appends") {
    auto buffer = from<SharedBuffer>("x");
    auto writer = BufferWriter(Ref(buffer));
    auto tail = from<SharedBuffer>("y");
    writer.write(tail);
    REQUIRE(buffer == "xy");
  }

  TEST_CASE("write_zero_bytes_does_nothing") {
    auto buffer = SharedBuffer();
    auto writer = BufferWriter(Ref(buffer));
    writer.write(SharedBuffer());
    REQUIRE(buffer.get_size() == 0);
  }
}
