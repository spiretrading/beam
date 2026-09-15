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

  TEST_CASE("shared_growth") {
    auto buffer = SharedBuffer();
    SUBCASE("initial_size") {
      buffer = SharedBuffer(3);
      buffer.write(0, "abc", 3);
    }
    SUBCASE("copied_data") {
      buffer = from<SharedBuffer>("abc");
    }
    auto original = buffer;
    REQUIRE(buffer.get_data() == original.get_data());
    append(buffer, "defgh", 5);
    REQUIRE(buffer == "abcdefgh");
    REQUIRE(original == "abc");
    auto copy = buffer;
    buffer = SharedBuffer();
    REQUIRE(copy == "abcdefgh");
    copy.write(0, "X", 1);
    REQUIRE(copy == "Xbcdefgh");
    REQUIRE(original == "abc");
  }

  TEST_CASE("slice") {
    auto buffer = from<SharedBuffer>("abcdef");
    auto slice = buffer.slice(1, 3);
    REQUIRE(slice == "bcd");
    REQUIRE(slice.get_data() == buffer.get_data() + 1);
    SUBCASE("write") {
      slice.write(0, "X", 1);
      REQUIRE(slice == "Xcd");
      REQUIRE(buffer == "abcdef");
    }
    SUBCASE("mutable_data") {
      slice.get_mutable_data()[0] = 'X';
      REQUIRE(slice == "Xcd");
      REQUIRE(buffer == "abcdef");
    }
    SUBCASE("growth") {
      append(slice, "xyz", 3);
      REQUIRE(slice == "bcdxyz");
      REQUIRE(buffer == "abcdef");
    }
    SUBCASE("source_mutation") {
      buffer.write(1, "X", 1);
      REQUIRE(slice == "bcd");
      REQUIRE(buffer == "aXcdef");
    }
    SUBCASE("nested_lifetime") {
      auto nested = slice.slice(1, 2);
      slice = SharedBuffer();
      buffer = SharedBuffer();
      REQUIRE(nested == "cd");
      append(nested, "xyz", 3);
      REQUIRE(nested == "cdxyz");
    }
    SUBCASE("empty") {
      auto empty = buffer.slice(buffer.get_size(), 0);
      REQUIRE(empty.get_size() == 0);
      REQUIRE(!empty.get_data());
      REQUIRE(SharedBuffer().slice(0, 0).get_size() == 0);
      append(empty, "x", 1);
      REQUIRE(empty == "x");
    }
    SUBCASE("bounds") {
      REQUIRE_THROWS_AS(buffer.slice(7, 0), std::out_of_range);
      REQUIRE_THROWS_AS(buffer.slice(6, 1), std::out_of_range);
      REQUIRE_THROWS_AS(buffer.slice(2, buffer.get_size()), std::out_of_range);
    }
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
