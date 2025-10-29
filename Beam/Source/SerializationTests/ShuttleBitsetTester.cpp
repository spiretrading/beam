#include <bitset>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleBitset.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleBitset") {
  TEST_CASE("single_bit") {
    test_round_trip_shuttle(std::bitset<1>(1ULL));
  }

  TEST_CASE("small_multiple_bits") {
    test_round_trip_shuttle(std::bitset<8>(0b10110010ULL));
  }

  TEST_CASE("all_bits_set") {
    test_round_trip_shuttle(std::bitset<16>((1ULL << 16) - 1ULL));
  }

  TEST_CASE("alternating_pattern") {
    test_round_trip_shuttle(std::bitset<32>(0xAAAAAAAAULL));
  }

  TEST_CASE("size_64_high_bit") {
    test_round_trip_shuttle(std::bitset<64>(1ULL << 63));
  }
}
