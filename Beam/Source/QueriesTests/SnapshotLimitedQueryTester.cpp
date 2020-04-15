#include <doctest/doctest.h>
#include "Beam/Queries/SnapshotLimitedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("SnapshotLimitedQuery") {
  TEST_CASE("default_constructor") {
    auto query = SnapshotLimitedQuery();
    REQUIRE(query.GetSnapshotLimit() == SnapshotLimit::None());
  }

  TEST_CASE("snapshot_limit_constructor") {
    auto unlimitedQuery = SnapshotLimitedQuery(SnapshotLimit::Unlimited());
    REQUIRE(unlimitedQuery.GetSnapshotLimit() == SnapshotLimit::Unlimited());
    auto headQuery = SnapshotLimitedQuery(
      SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
    REQUIRE(headQuery.GetSnapshotLimit() ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
    auto tailQuery = SnapshotLimitedQuery(
      SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
    REQUIRE(tailQuery.GetSnapshotLimit() ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
  }

  TEST_CASE("set_snapshot_limit") {
    auto query = SnapshotLimitedQuery();
    REQUIRE(query.GetSnapshotLimit() != SnapshotLimit::Unlimited());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    REQUIRE(query.GetSnapshotLimit() == SnapshotLimit::Unlimited());
    query.SetSnapshotLimit(SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
    REQUIRE(query.GetSnapshotLimit() ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
    query.SetSnapshotLimit(SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
    REQUIRE(query.GetSnapshotLimit() ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
  }
}
