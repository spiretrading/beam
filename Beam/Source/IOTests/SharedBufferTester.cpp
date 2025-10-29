#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("SharedBuffer") {
  TEST_CASE("default") {
    auto buffer = SharedBuffer();
    REQUIRE(buffer.get_size() == 0);
    REQUIRE(!buffer.get_data());
  }

  TEST_CASE("initial_size") {
    auto buffer = SharedBuffer(16);
    REQUIRE(buffer.get_size() == 16);
    REQUIRE(buffer.get_data());
    REQUIRE(buffer.get_mutable_data());
  }

  TEST_CASE("copy_data") {
    auto source = std::string("hello");
    auto buffer = SharedBuffer(source.data(), source.size());
    REQUIRE(buffer == source);
  }

  TEST_CASE("copy_on_write") {
    auto source = std::string("abcdef");
    auto a = SharedBuffer(source.data(), source.size());
    auto b = a;
    auto ptr = b.get_mutable_data();
    ptr[0] = 'X';
    REQUIRE(a.get_data()[0] == 'a');
    REQUIRE(b.get_data()[0] == 'X');
  }

  TEST_CASE("move") {
    auto original = SharedBuffer("xyz", 3);
    auto moved = SharedBuffer(std::move(original));
    REQUIRE(moved == "xyz");
    REQUIRE(original.get_size() == 0);
  }

  TEST_CASE("equality") {
    auto a = SharedBuffer("same", 4);
    auto b = SharedBuffer("same", 4);
    auto c = SharedBuffer("diff", 4);
    REQUIRE(a == b);
    REQUIRE(a != c);
  }

  TEST_CASE("append_and_write") {
    auto buffer = SharedBuffer();
    append(buffer, "abc", 3);
    REQUIRE(buffer == "abc");
    buffer.write(1, "Z", 1);
    REQUIRE(buffer == "aZc");
    append(buffer, "d", 1);
    REQUIRE(buffer == "aZcd");
  }

  TEST_CASE("shrink") {
    auto buffer = SharedBuffer("abcdef", 6);
    buffer.shrink(2);
    REQUIRE(buffer == "abcd");
  }

  TEST_CASE("reserve_and_grow") {
    auto buffer = SharedBuffer();
    reserve(buffer, 5);
    REQUIRE(buffer.get_size() == 5);
    REQUIRE(buffer.get_mutable_data());
    auto before = buffer.get_size();
    buffer.grow(3);
    REQUIRE(buffer.get_size() == before + 3);
  }
}
