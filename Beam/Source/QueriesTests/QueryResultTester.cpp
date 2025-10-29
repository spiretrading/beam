#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Queries/QueryResult.hpp"
#include "Beam/QueriesTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("QueryResult") {
  TEST_CASE("default_constructor") {
    auto result = QueryResult<int>();
    REQUIRE(result.m_id == -1);
    REQUIRE(result.m_snapshot.empty());
  }

  TEST_CASE("constructor") {
    auto snapshot = std::vector<std::string>{"alpha", "beta"};
    auto result = QueryResult(7, snapshot);
    REQUIRE(result.m_id == 7);
    REQUIRE(result.m_snapshot == snapshot);
  }

  TEST_CASE("stream") {
    auto ss = std::stringstream();
    auto query = QueryResult(5, std::vector{10, 20, 30});
    test_round_trip_shuttle(query);
  }
}
