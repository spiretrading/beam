#include <doctest/doctest.h>
#include <boost/variant/variant.hpp>
#include "Beam/Serialization/ShuttleVariant.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

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
