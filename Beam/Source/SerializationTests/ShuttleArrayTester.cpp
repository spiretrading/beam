#include <array>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleArray.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleArray") {
  TEST_CASE("zero_length") {
    test_round_trip_shuttle(std::array<int, 0>());
  }

  TEST_CASE("double_sequence") {
    auto value = std::array{1.1, 2.2, 3.3, 4.4};
    test_round_trip_shuttle(value);
  }

  TEST_CASE("int_sequence") {
    auto value = std::array{5, 4, 3, 2, 1};
    test_round_trip_shuttle(value);
  }

  TEST_CASE("string_sequence") {
    auto value = std::array<std::string, 2>{"hello", ""};
    test_round_trip_shuttle(value);
  }

  TEST_CASE("single_element") {
    auto value = std::array<int, 1>{42};
    test_round_trip_shuttle(value);
  }
}
