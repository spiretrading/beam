#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ShuttleQueryTypes.hpp"
#include "Beam/Queries/Value.hpp"
#include "Beam/QueriesTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("Value") {
  TEST_CASE("construct_from_int") {
    auto value = Value(42);
    REQUIRE(value.get_type() == typeid(int));
    REQUIRE(value.as<int>() == 42);
  }

  TEST_CASE("construct_from_string") {
    auto value = Value("hello");
    REQUIRE(value.get_type() == typeid(std::string));
    REQUIRE(value.as<std::string>() == "hello");
  }

  TEST_CASE("copy") {
    auto original = Value("world");
    auto copy = original;
    REQUIRE(copy.get_type() == typeid(std::string));
    REQUIRE(copy.as<std::string>() == "world");
    REQUIRE(original.as<std::string>() == "world");
  }

  TEST_CASE("equality") {
    auto a = Value(3.14);
    auto b = Value(3.14);
    auto c = Value(2.71);
    auto d = Value("3.14");
    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(d == "3.14");
    REQUIRE(d != a);
  }

  TEST_CASE("bad_cast") {
    auto value = Value(100);
    REQUIRE_THROWS_AS(value.as<double>(), std::bad_cast);
  }

  TEST_CASE("stream") {
    auto value = Value(123);
    auto stream = std::stringstream();
    stream << value;
    REQUIRE(stream.str() == "123");
    test_query_round_trip_shuttle(value);
  }
}
