#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/StaticBuffer.hpp"

using namespace Beam;

TEST_SUITE("StaticBuffer") {
  TEST_CASE("default_construct") {
    auto buffer = StaticBuffer<16>();
    REQUIRE(buffer.get_size() == 0);
    REQUIRE(!buffer.get_data());
  }

  TEST_CASE("construct_from_raw_data") {
    auto source = std::string("hello");
    auto buffer = StaticBuffer<8>(source.data(), source.size());
    REQUIRE(buffer == source);
  }

  TEST_CASE("construct_from_buffer") {
    auto source = std::string("world");
    auto shared = SharedBuffer(source.data(), source.size());
    auto buffer = StaticBuffer<8>(shared);
    REQUIRE(buffer == shared);
  }

  TEST_CASE("copy_constructor") {
    auto buffer = StaticBuffer<8>("abc", 3);
    auto copy = StaticBuffer<8>(buffer);
    REQUIRE(copy == buffer);
    auto data = copy.get_mutable_data();
    REQUIRE(data);
    data[0] = 'X';
    REQUIRE(buffer.get_data()[0] == 'a');
    REQUIRE(copy.get_data()[0] == 'X');
  }

  TEST_CASE("grow") {
    auto buffer = StaticBuffer<16>();
    auto before = buffer.get_size();
    buffer.grow(3);
    REQUIRE(buffer.get_size() == before + 3);
  }

  TEST_CASE("shrink_operations") {
    auto a = StaticBuffer<8>("abcdef", 6);
    a.shrink(2);
    REQUIRE(a == "abcd");
  }

  TEST_CASE("write_operations") {
    auto buffer = StaticBuffer<8>();
    buffer.write(0, "abc", 3);
    REQUIRE(buffer == "abc");
    auto small = StaticBuffer<4>();
    REQUIRE_THROWS_AS(small.write(2, "xyz", 3), std::out_of_range);
  }

  TEST_CASE("grow") {
    auto buffer = StaticBuffer<4>();
    REQUIRE(buffer.grow(5) == 4);
  }

  TEST_CASE("assignment_operations") {
    auto shared = SharedBuffer("assign", 6);
    auto buffer = StaticBuffer<8>();
    buffer = shared;
    REQUIRE(buffer == shared);
    auto other = StaticBuffer<8>("zzzz", 4);
    other = buffer;
    REQUIRE(other == buffer);
  }
}
