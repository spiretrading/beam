#include <variant>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleVariant.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleVariant") {
  TEST_CASE("json_format") {
    auto single = std::variant<int>(123);
    auto single_json = std::string("{\"__version\":0,\"value\":123}");
    REQUIRE(to_json(single) == single_json);
    REQUIRE(from_json<std::variant<int>>(single_json) == single);
    auto multiple = std::variant<int, std::string>("hello");
    auto multiple_json = std::string(
      "{\"__version\":0,\"which\":1,\"value\":\"hello\"}");
    REQUIRE(to_json(multiple) == multiple_json);
    REQUIRE(from_json<std::variant<int, std::string>>(multiple_json) ==
      multiple);
  }

  TEST_CASE("duplicate_types") {
    test_round_trip_shuttle(std::variant<int, std::string, int>(
      std::in_place_index<2>, 123));
  }

  TEST_CASE("single_type") {
    test_round_trip_shuttle(std::variant<int>(123));
  }

  TEST_CASE("two_types") {
    test_round_trip_shuttle(std::variant<int, std::string>(123));
    test_round_trip_shuttle(std::variant<int, std::string>("hello"));
  }

  TEST_CASE("three_types") {
    test_round_trip_shuttle(std::variant<int, std::string, double>(24));
    test_round_trip_shuttle(std::variant<int, std::string, double>("hello"));
    test_round_trip_shuttle(std::variant<int, std::string, double>(3.1415));
  }

  TEST_CASE("many_types") {
    using V = std::variant<bool, char, int, double, std::string>;
    test_round_trip_shuttle(V(true));
    test_round_trip_shuttle(V('x'));
    test_round_trip_shuttle(V(42));
    test_round_trip_shuttle(V(2.718));
    test_round_trip_shuttle(V(std::string("test")));
  }
}
