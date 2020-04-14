#include <doctest/doctest.h>
#include "Beam/Queries/RangedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("RangedQuery") {
  TEST_CASE("default_constructor") {
    auto query = RangedQuery();
    REQUIRE(query.GetRange() == Range::Empty());
  }

  TEST_CASE("range_constructor") {
    auto totalRangeQuery = RangedQuery(Range::Total());
    REQUIRE(totalRangeQuery.GetRange() == Range::Total());
    auto realTimeRangeQuery = RangedQuery(Range::RealTime());
    REQUIRE(realTimeRangeQuery.GetRange() == Range::RealTime());
    auto sequenceRangeQuery = RangedQuery(Range(Sequence(1), Sequence(2)));
    REQUIRE(sequenceRangeQuery.GetRange() == Range(Sequence(1), Sequence(2)));
  }

  TEST_CASE("set_range") {
    auto query = RangedQuery();
    REQUIRE(query.GetRange() != Range::Total());
    query.SetRange(Range::Total());
    REQUIRE(query.GetRange() == Range::Total());
    query.SetRange(Range::RealTime());
    REQUIRE(query.GetRange() == Range::RealTime());
    query.SetRange(Range(Sequence(1), Sequence(2)));
    REQUIRE(query.GetRange() == Range(Sequence(1), Sequence(2)));
  }
}
