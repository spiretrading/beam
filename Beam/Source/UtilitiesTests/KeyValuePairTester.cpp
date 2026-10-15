#include <string>
#include <doctest/doctest.h>
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  struct Number {
    int m_value;
  };

  struct StreamValue {
    int m_value;
  };

  std::ostream& operator <<(std::ostream& out, const StreamValue& value) {
    return out << "stream:" << value.m_value;
  }
}

template<>
class std::formatter<Number> : public std::formatter<int> {
  public:
    auto format(const Number& value, auto& context) const {
      return std::formatter<int>::format(value.m_value, context);
    }
};

TEST_SUITE("KeyValuePair") {
  TEST_CASE("format") {
    auto pair = KeyValuePair(std::string("key"), Number(42));
    REQUIRE(std::format("{}", pair) == "(key 42)");
    REQUIRE(std::format("{:*>10}", pair) == "**(key 42)");
    REQUIRE(std::format("{:.4}", pair) == "(key");
    REQUIRE(to_string(pair) == "(key 42)");
    REQUIRE(std::format("{}", KeyValuePair(true, 1.23456789)) ==
      "(true 1.23456789)");
    REQUIRE(std::format("{}", KeyValuePair(7, pair)) == "(7 (key 42))");
    REQUIRE_FALSE((std::formattable<KeyValuePair<int, StreamValue>, char>));
    REQUIRE(to_string(KeyValuePair(7, StreamValue(42))) == "(7 stream:42)");
  }

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
    REQUIRE(to_string(pair) == "(5 value)");
    test_round_trip_shuttle(pair);
  }
}
