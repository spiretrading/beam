#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/SuffixBuffer.hpp"

using namespace Beam;

TEST_SUITE("SuffixBuffer") {
  TEST_CASE("zero_offset") {
    auto message = std::string("hello");
    auto buffer = from<SharedBuffer>(message);
    auto suffix = SuffixBuffer(Ref(buffer), 0);
    REQUIRE(suffix == buffer);
  }

  TEST_CASE("middle_offset") {
    auto message = std::string("hello world");
    auto buffer = from<SharedBuffer>(message);
    auto suffix = SuffixBuffer(Ref(buffer), 6);
    REQUIRE(suffix == "world");
  }

  TEST_CASE("mutable_view") {
    auto buffer = from<SharedBuffer>("abcde");
    auto suffix = SuffixBuffer(Ref(buffer), 2);
    auto ptr = suffix.get_mutable_data();
    REQUIRE(ptr);
    ptr[0] = 'X';
    REQUIRE(buffer.get_data()[2] == 'X');
    REQUIRE(suffix.get_data()[0] == 'X');
  }

  TEST_CASE("grow_reserve_affects_underlying") {
    auto buffer = from<SharedBuffer>("ab");
    auto suffix = SuffixBuffer(Ref(buffer), 1);
    auto before_size = buffer.get_size();
    REQUIRE(before_size == 2);
    suffix.grow(3);
    REQUIRE(buffer.get_size() == before_size + 3);
    REQUIRE(suffix.get_size() == (buffer.get_size() - 1));
  }

  TEST_CASE("shrink_end_maps_to_underlying_shrink") {
    auto buffer = from<SharedBuffer>("abcdef");
    auto suffix = SuffixBuffer(Ref(buffer), 2);
    suffix.shrink(2);
    REQUIRE(buffer.get_size() == 4);
    REQUIRE(suffix == "cd");
  }

  TEST_CASE("reset_removes_suffix") {
    auto buffer = from<SharedBuffer>("hello");
    auto suffix = SuffixBuffer(Ref(buffer), 2);
    reset(suffix);
    REQUIRE(buffer.get_size() == 2);
    REQUIRE(suffix.get_size() == 0);
  }
}
