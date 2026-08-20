#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/Queries/ShuttleQueryTypes.hpp"
#include "Beam/Queries/Value.hpp"
#include "Beam/QueriesTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;

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
    REQUIRE(to_string(Value(123)) == "123");
  }

  TEST_CASE("shuttle") {
    test_query_round_trip_shuttle(Value(true));
    test_query_round_trip_shuttle(Value('a'));
    test_query_round_trip_shuttle(Value(123));
    test_query_round_trip_shuttle(Value(3.14));
    test_query_round_trip_shuttle(Value(std::uint64_t(18446744073709551615)));
    test_query_round_trip_shuttle(Value("hello"));
    test_query_round_trip_shuttle(
      Value(time_from_string("2020-01-02 03:04:05")));
    test_query_round_trip_shuttle(Value(duration_from_string("01:30:00")));
  }
}
