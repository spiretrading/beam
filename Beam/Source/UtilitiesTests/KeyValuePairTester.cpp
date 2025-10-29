#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("KeyValuePair") {
  TEST_CASE("default_construction") {
    auto pair = KeyValuePair<int, std::string>();
    REQUIRE(pair.m_key == 0);
    REQUIRE(pair.m_value == "");
  }

  TEST_CASE("value_construction") {
    auto pair = KeyValuePair(42, std::string("hello"));
    REQUIRE(pair.m_key == 42);
    REQUIRE(pair.m_value == "hello");
    REQUIRE(pair == pair);
  }

  TEST_CASE("equality_different_keys") {
    auto pair1 = KeyValuePair(10, std::string("test"));
    auto pair2 = KeyValuePair(20, std::string("test"));
    REQUIRE_FALSE(pair1 == pair2);
  }

  TEST_CASE("equality_different_values") {
    auto pair1 = KeyValuePair(10, std::string("test"));
    auto pair2 = KeyValuePair(10, std::string("other"));
    REQUIRE_FALSE(pair1 == pair2);
  }

  TEST_CASE("stream") {
    auto pair = KeyValuePair(5, std::string("value"));
    auto ss = std::stringstream();
    ss << pair;
    REQUIRE(ss.str() == "(5 value)");
    test_round_trip_shuttle(pair);
  }
}
