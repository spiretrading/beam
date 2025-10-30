#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/ShuttleQueryTypes.hpp"
#include "Beam/QueriesTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("BasicQuery") {
  TEST_CASE("default_construction") {
    auto query = BasicQuery<int>();
    REQUIRE(query.get_index() == 0);
    REQUIRE(query.get_range() == Range::EMPTY);
    REQUIRE(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::HEAD);
    REQUIRE(query.get_snapshot_limit().get_size() == 0);
    REQUIRE(query.get_interruption_policy() == InterruptionPolicy::BREAK_QUERY);
    REQUIRE(query.get_filter().get_type() == typeid(bool));
  }

  TEST_CASE("make_current_query") {
    auto query = make_current_query(5);
    REQUIRE(query.get_index() == 5);
    REQUIRE(query.get_range() == Range::TOTAL);
    REQUIRE(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::TAIL);
    REQUIRE(query.get_snapshot_limit().get_size() == 1);
    REQUIRE(
      query.get_interruption_policy() == InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_latest_query") {
    auto query = make_latest_query(42);
    REQUIRE(query.get_index() == 42);
    REQUIRE(query.get_range() == Range::HISTORICAL);
    REQUIRE(query.get_snapshot_limit().get_type() == SnapshotLimit::Type::TAIL);
    REQUIRE(query.get_snapshot_limit().get_size() == 1);
    REQUIRE(query.get_interruption_policy() == InterruptionPolicy::BREAK_QUERY);
  }

  TEST_CASE("make_real_time_query") {
    auto query = make_real_time_query(7);
    REQUIRE(query.get_index() == 7);
    REQUIRE(query.get_range() == Range::REAL_TIME);
    REQUIRE(
      query.get_interruption_policy() == InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("stream") {
    auto query = BasicQuery<int>();
    query.set_index(7);
    auto output = to_string(query);
    test_query_round_trip_shuttle(query, [&] (auto&& received) {
      REQUIRE(received.get_index() == 7);
      REQUIRE(received.get_range() == Range::EMPTY);
      REQUIRE(received.get_snapshot_limit().get_type() ==
        SnapshotLimit::Type::HEAD);
      REQUIRE(received.get_snapshot_limit().get_size() == 0);
      REQUIRE(received.get_interruption_policy() ==
        InterruptionPolicy::BREAK_QUERY);
      REQUIRE(received.get_filter().get_type() == typeid(bool));
    });
  }
}
