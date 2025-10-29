#include <boost/multiprecision/cpp_dec_float.hpp>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleDecimal.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace boost;
using namespace boost::multiprecision;
using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleDecimal") {
  TEST_CASE("basic_decimal") {
    test_round_trip_shuttle(
      cpp_dec_float_50("123.456789012345678901234567890"));
  }

  TEST_CASE("negative_value") {
    test_round_trip_shuttle(
      cpp_dec_float_50("-0.000001234567890123456789"));
  }

  TEST_CASE("high_precision") {
    test_round_trip_shuttle(
      cpp_dec_float_50("3.14159265358979323846264338327950288419716939937510"));
  }

  TEST_CASE("zero_and_negative_zero") {
    test_round_trip_shuttle(cpp_dec_float_50("0"));
    test_round_trip_shuttle(cpp_dec_float_50("-0"));
  }

  TEST_CASE("large_exponent") {
    test_round_trip_shuttle(cpp_dec_float_50("1e+50"));
  }

  TEST_CASE("small_exponent") {
    test_round_trip_shuttle(cpp_dec_float_50("1e-50"));
  }

  TEST_CASE("very_small_value") {
    test_round_trip_shuttle(
      cpp_dec_float_50("1.2345678901234567890123456789e-100"));
  }
}
