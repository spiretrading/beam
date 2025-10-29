#include <string>
#include <unordered_set>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleUnorderedSet.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleUnorderedSet") {
  TEST_CASE("zero_length") {
    test_round_trip_shuttle(std::unordered_set<int>());
  }

  TEST_CASE("int_set") {
    test_round_trip_shuttle(std::unordered_set{5, 4, 3, 2, 1});
  }

  TEST_CASE("duplicate_elements") {
    test_round_trip_shuttle(std::unordered_set{1, 1, 2, 2, 3});
  }

  TEST_CASE("double_set") {
    test_round_trip_shuttle(std::unordered_set{1.1, 2.2, 3.3, 4.4});
  }

  TEST_CASE("string_set") {
    test_round_trip_shuttle(std::unordered_set<std::string>{"hello", ""});
  }

  TEST_CASE("single_element") {
    test_round_trip_shuttle(std::unordered_set{42});
  }
}
