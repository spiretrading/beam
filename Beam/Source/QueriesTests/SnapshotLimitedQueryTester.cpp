#include <doctest/doctest.h>
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("SnapshotLimitedQuery") {
  TEST_CASE("default_constructor") {
    auto query = SnapshotLimitedQuery();
    REQUIRE(query.get_snapshot_limit() == SnapshotLimit::NONE);
  }

  TEST_CASE("snapshot_limit_constructor") {
    auto unlimited_query = SnapshotLimitedQuery(SnapshotLimit::UNLIMITED);
    REQUIRE(unlimited_query.get_snapshot_limit() == SnapshotLimit::UNLIMITED);
    auto head_query = SnapshotLimitedQuery(SnapshotLimit::from_head(100));
    REQUIRE(head_query.get_snapshot_limit() == SnapshotLimit::from_head(100));
    auto tail_query = SnapshotLimitedQuery(SnapshotLimit::from_tail(200));
    REQUIRE(tail_query.get_snapshot_limit() == SnapshotLimit::from_tail(200));
  }

  TEST_CASE("set_snapshot_limit") {
    auto query = SnapshotLimitedQuery();
    REQUIRE(query.get_snapshot_limit() != SnapshotLimit::UNLIMITED);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    REQUIRE(query.get_snapshot_limit() == SnapshotLimit::UNLIMITED);
    query.set_snapshot_limit(SnapshotLimit::from_head(100));
    REQUIRE(query.get_snapshot_limit() == SnapshotLimit::from_head(100));
    query.set_snapshot_limit(SnapshotLimit::from_tail(200));
    REQUIRE(query.get_snapshot_limit() == SnapshotLimit::from_tail(200));
  }

  TEST_CASE("stream") {
    auto query = SnapshotLimitedQuery(SnapshotLimit::from_head(50));
    auto ss = std::stringstream();
    ss << query;
    REQUIRE(ss.str() == "(HEAD 50)");
    test_round_trip_shuttle(query);
  }
}
