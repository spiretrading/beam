#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleRecord.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  BEAM_DEFINE_RECORD(ZeroField);

  BEAM_DEFINE_RECORD(SingleField,
    (int, value));

  BEAM_DEFINE_RECORD(FiveField,
    (char, letter),
    (int, number),
    (double, amount),
    (std::string, text),
    (long long, big_number));
}

TEST_SUITE("ShuttleRecord") {
  TEST_CASE("zero_fields") {
    test_round_trip_shuttle(ZeroField(), [&] (auto&&) {
      REQUIRE(true);
    });
  }

  TEST_CASE("single_field") {
    test_round_trip_shuttle([&] {
      return SingleField(42);
    }(), [&] (auto&& received) {
      REQUIRE(received.value == 42);
    });
  }

  TEST_CASE("five_fields") {
    test_round_trip_shuttle([&] {
      return FiveField('z', 7, 3.14, "hello", 123456789LL);
    }(), [&] (auto&& received) {
      REQUIRE(received.letter == 'z');
      REQUIRE(received.number == 7);
      REQUIRE(received.amount == 3.14);
      REQUIRE(received.text == "hello");
      REQUIRE(received.big_number == 123456789LL);
    });
  }
}
