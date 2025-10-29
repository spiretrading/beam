#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/BufferRef.hpp"

using namespace Beam;

TEST_SUITE("BufferRef") {
  TEST_CASE("data_and_size") {
    auto source = std::string("hello");
    auto buffer = SharedBuffer(source.data(), source.size());
    auto ref = BufferRef(buffer);
    REQUIRE(ref == buffer);
  }

  TEST_CASE("nesting") {
    auto buffer = SharedBuffer("nested", 6);
    auto parent = BufferRef(buffer);
    auto child = BufferRef(parent);
    REQUIRE(child == buffer);
    auto ptr = child.get_mutable_data();
    REQUIRE(ptr);
    ptr[0] = 'N';
    REQUIRE(buffer.get_data()[0] == 'N');
    REQUIRE(child.get_data()[0] == 'N');
  }

  TEST_CASE("mutable_view") {
    auto buffer = SharedBuffer("abcde", 5);
    auto ref = BufferRef(buffer);
    auto data = ref.get_mutable_data();
    REQUIRE(data);
    data[0] = 'X';
    REQUIRE(buffer.get_data()[0] == 'X');
    REQUIRE(ref.get_data()[0] == 'X');
  }

  TEST_CASE("grow_and_reserve_affect_underlying") {
    auto buffer = SharedBuffer("ab", 2);
    auto ref = BufferRef(buffer);
    auto before_size = buffer.get_size();
    REQUIRE(before_size == 2);
    ref.grow(3);
    REQUIRE(buffer.get_size() == before_size + 3);
    REQUIRE(ref.get_size() == buffer.get_size());
  }

  TEST_CASE("shrink_and_write") {
    auto buffer = SharedBuffer("abcdef", 6);
    auto ref = BufferRef(buffer);
    ref.shrink(2);
    REQUIRE(buffer == "abcd");
    ref.write(1, "Z", 1);
    REQUIRE(buffer == "aZcd");
  }

  TEST_CASE("reset") {
    auto buffer2 = SharedBuffer("hello", 5);
    auto ref2 = BufferRef(buffer2);
    reset(ref2);
    REQUIRE(buffer2.get_size() == 0);
  }
}
