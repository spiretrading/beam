#include <doctest/doctest.h>
#include "Beam/TimeService/LocalTimeClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("LocalTimeClient") {
  TEST_CASE("get_time_returns_utc") {
    auto client = LocalTimeClient();
    auto time = client.get_time();
    auto current = microsec_clock::universal_time();
    auto difference = time - current;
    auto tolerance = minutes(1);
    REQUIRE(difference >= -tolerance);
    REQUIRE(difference <= tolerance);
  }
}
