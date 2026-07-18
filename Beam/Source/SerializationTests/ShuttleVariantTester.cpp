module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <variant>
#include <boost/variant/variant.hpp>
#include "Beam/Serialization/ShuttleVariant.hpp"

module Beam;

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

TEST_SUITE("ShuttleVariant") {
  TEST_CASE("single_type") {
    test_round_trip_shuttle(variant<int>(123));
  }

  TEST_CASE("two_types") {
    test_round_trip_shuttle(variant<int, std::string>(123));
    test_round_trip_shuttle(variant<int, std::string>("hello"));
  }

  TEST_CASE("three_types") {
    test_round_trip_shuttle(variant<int, std::string, double>(24));
    test_round_trip_shuttle(variant<int, std::string, double>("hello"));
    test_round_trip_shuttle(variant<int, std::string, double>(3.1415));
  }
}

TEST_SUITE("ShuttleStdVariant") {
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
