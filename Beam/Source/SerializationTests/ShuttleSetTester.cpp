#include <set>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleSet.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleSet") {
  TEST_CASE("zero_length") {
    test_round_trip_shuttle(std::set<int>());
  }

  TEST_CASE("int_set") {
    test_round_trip_shuttle(std::set{5, 4, 3, 2, 1});
  }

  TEST_CASE("duplicate_elements") {
    test_round_trip_shuttle(std::set{1, 1, 2, 2, 3});
  }

  TEST_CASE("double_set") {
    test_round_trip_shuttle(std::set{1.1, 2.2, 3.3, 4.4});
  }

  TEST_CASE("string_set") {
    test_round_trip_shuttle(std::set<std::string>{"hello", ""});
  }

  TEST_CASE("single_element") {
    test_round_trip_shuttle(std::set{42});
  }
}
