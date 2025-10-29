#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("RangedQuery") {
  TEST_CASE("default_constructor") {
    auto query = RangedQuery();
    REQUIRE(query.get_range() == Range::EMPTY);
  }

  TEST_CASE("range_constructor") {
    auto total_range_query = RangedQuery(Range::TOTAL);
    REQUIRE(total_range_query.get_range() == Range::TOTAL);
    auto real_time_range_query = RangedQuery(Range::REAL_TIME);
    REQUIRE(real_time_range_query.get_range() == Range::REAL_TIME);
    auto sequence_range_query = RangedQuery(Range(Sequence(1), Sequence(2)));
    REQUIRE(
      sequence_range_query.get_range() == Range(Sequence(1), Sequence(2)));
  }

  TEST_CASE("set_range") {
    auto query = RangedQuery();
    REQUIRE(query.get_range() != Range::TOTAL);
    query.set_range(Range::TOTAL);
    REQUIRE(query.get_range() == Range::TOTAL);
    query.set_range(Range::REAL_TIME);
    REQUIRE(query.get_range() == Range::REAL_TIME);
    query.set_range(Sequence(1), Sequence(2));
    REQUIRE(query.get_range() == Range(Sequence(1), Sequence(2)));
  }

  TEST_CASE("stream") {
    auto query = RangedQuery(Range::TOTAL);
    auto ss = std::stringstream();
    ss << query;
    REQUIRE(ss.str() == "Total");
    test_round_trip_shuttle(query);
  }
}
