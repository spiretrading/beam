#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Collections/EnumSet.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  BEAM_ENUM(Color,
    RED,
    GREEN,
    BLUE)
}

TEST_SUITE("EnumSet") {
  TEST_CASE("default_construct_is_empty") {
    auto set = EnumSet<Color>();
    REQUIRE(set.get_bitset().none());
    REQUIRE(set.test(Color::Type::NONE));
    REQUIRE(!set.test(Color::Type::RED));
  }

  TEST_CASE("construct_from_single_value") {
    auto set = EnumSet<Color>(Color::Type::GREEN);
    REQUIRE(set.test(Color::Type::GREEN));
    REQUIRE(!set.test(Color::Type::RED));
  }

  TEST_CASE("set_and_reset_member") {
    auto set = EnumSet<Color>();
    set.set(Color::Type::BLUE);
    REQUIRE(set.test(Color::Type::BLUE));
    set.reset(Color::Type::BLUE);
    REQUIRE(!set.test(Color::Type::BLUE));
  }

  TEST_CASE("merge_sets_with_set") {
    auto left = EnumSet<Color>(Color::Type::RED);
    auto right = EnumSet<Color>(Color::Type::BLUE);
    left.set(right);
    REQUIRE(left.test(Color::Type::RED));
    REQUIRE(left.test(Color::Type::BLUE));
  }

  TEST_CASE("bitwise_operators") {
    auto left = EnumSet<Color>(Color::Type::RED);
    auto right = EnumSet<Color>(Color::Type::GREEN);
    auto result = left | right;
    REQUIRE(result.test(Color::Type::RED));
    REQUIRE(result.test(Color::Type::GREEN));
    auto intersection = result & right;
    REQUIRE(!intersection.test(Color::Type::RED));
    REQUIRE(intersection.test(Color::Type::GREEN));
    auto symmetric_difference = result ^ right;
    REQUIRE(symmetric_difference.test(Color::Type::RED));
    REQUIRE(!symmetric_difference.test(Color::Type::GREEN));
  }

  TEST_CASE("from_bitmask_and_conversion") {
    auto mask_set =
      EnumSet<Color>::from_bitmask(static_cast<std::uint32_t>(0b101));
    REQUIRE(mask_set.test(Color::Type::RED));
    REQUIRE(!mask_set.test(Color::Type::GREEN));
    REQUIRE(mask_set.test(Color::Type::BLUE));
    auto bitset = mask_set.get_bitset();
    REQUIRE(bitset.test(0));
    REQUIRE(!bitset.test(1));
    REQUIRE(bitset.test(2));
  }

  TEST_CASE("stream") {
    auto set = EnumSet<Color>();
    set.set(Color::Type::RED);
    set.set(Color::Type::BLUE);
    auto out = std::stringstream();
    out << set;
    auto expected =
      std::bitset<Color::COUNT>(static_cast<unsigned long>(0b101)).to_string();
    REQUIRE(out.str() == expected);
    test_round_trip_shuttle(set);
  }
}
