#include <doctest/doctest.h>
#include "Beam/QueriesTests/SnapshotLimitedQueryTester.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;

TEST_SUITE("SnapshotLimitedQuery") {
  TEST_CASE("default_constructor") {
    SnapshotLimitedQuery query;
    REQUIRE(query.GetSnapshotLimit() == SnapshotLimit::None());
  }

  TEST_CASE("snapshot_limit_constructor") {
    SnapshotLimitedQuery unlimitedQuery(SnapshotLimit::Unlimited());
    REQUIRE(unlimitedQuery.GetSnapshotLimit() ==
      SnapshotLimit::Unlimited());
    SnapshotLimitedQuery headQuery(SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
    REQUIRE(headQuery.GetSnapshotLimit() ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 100));
    SnapshotLimitedQuery tailQuery(SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
    REQUIRE(tailQuery.GetSnapshotLimit() ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 200));
  }

  TEST_CASE("set_snapshot_limit") {
    SnapshotLimitedQuery query;
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
