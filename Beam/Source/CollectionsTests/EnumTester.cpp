#include <doctest/doctest.h>
#include "Beam/Collections/Enum.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  BEAM_ENUM(Fruit,
    APPLE,
    BANANA,
    CHERRY);
}

TEST_SUITE("EnumTester") {
  TEST_CASE("default_value") {
    auto value = Fruit();
    REQUIRE(value == Fruit::Type::NONE);
  }

  TEST_CASE("construct_from_type") {
    auto value = Fruit(Fruit::Type::BANANA);
    REQUIRE(value == Fruit::Type::BANANA);
  }

  TEST_CASE("construct_from_int") {
    auto value = Fruit(2);
    REQUIRE(value == Fruit::Type::CHERRY);
  }

  TEST_CASE("negative_int_constructs_none") {
    auto value = Fruit(-1);
    REQUIRE(value == Fruit::Type::NONE);
  }

  TEST_CASE("conversion_to_underlying_type") {
    auto converted = static_cast<Fruit::Type>(Fruit(Fruit::Type::APPLE));
    REQUIRE(converted == Fruit::Type::APPLE);
  }

  TEST_CASE("comparisons") {
    auto a = Fruit(Fruit::Type::APPLE);
    auto b = Fruit(Fruit::Type::BANANA);
    auto c = Fruit(Fruit::Type::CHERRY);
    REQUIRE(a < Fruit::Type::BANANA);
    REQUIRE(!(c < Fruit::Type::BANANA));
    REQUIRE(a != Fruit::Type::BANANA);
    REQUIRE(a == Fruit::Type::APPLE);
  }

  TEST_CASE("count_matches_number_of_members") {
    REQUIRE(Fruit::COUNT == 3);
  }

  TEST_CASE("streaming") {
    REQUIRE(to_string(Fruit::NONE) == "NONE");
    REQUIRE(to_string(Fruit(Fruit::NONE)) == "NONE");
    REQUIRE(to_string(Fruit::APPLE) == "APPLE");
    REQUIRE(to_string(Fruit(Fruit::APPLE)) == "APPLE");
    test_round_trip_shuttle(Fruit::NONE);
    test_round_trip_shuttle(Fruit(Fruit::NONE));
    test_round_trip_shuttle(Fruit::APPLE);
    test_round_trip_shuttle(Fruit(Fruit::BANANA));
  }

  TEST_CASE("from_string") {
    REQUIRE(Fruit::from("APPLE") == Fruit::Type::APPLE);
    REQUIRE(Fruit::from("BANANA") == Fruit::Type::BANANA);
    REQUIRE(Fruit::from("CHERRY") == Fruit::Type::CHERRY);
    REQUIRE(Fruit::from("INVALID") == Fruit::Type::NONE);
  }

  TEST_CASE("std_hash_matches_underlying_value") {
    auto value = Fruit(Fruit::Type::BANANA);
    auto h = std::hash<Fruit>()(value);
    REQUIRE(h == static_cast<std::size_t>(Fruit::Type::BANANA));
  }
}
