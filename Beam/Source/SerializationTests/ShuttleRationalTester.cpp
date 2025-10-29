#include <boost/rational.hpp>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleRational.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

TEST_SUITE("ShuttleRational") {
  TEST_CASE("simple_fraction") {
    test_round_trip_shuttle(rational<int>(3, 4));
  }

  TEST_CASE("normalization") {
    test_round_trip_shuttle(rational<int>(2, 4));
  }

  TEST_CASE("negative_numerator") {
    test_round_trip_shuttle(rational<int>(-3, 4));
  }

  TEST_CASE("negative_denominator") {
    test_round_trip_shuttle(rational<int>(3, -4));
  }

  TEST_CASE("zero_numerator") {
    test_round_trip_shuttle(rational<int>(0, 5));
  }

  TEST_CASE("large_values") {
    test_round_trip_shuttle(rational<long long>(123456789LL, 98765432LL));
  }
}
