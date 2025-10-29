#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/TimeService/FixedTimeClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("FixedTimeClient") {
  TEST_CASE("default_constructor") {
    auto client = FixedTimeClient();
    client.close();
  }

  TEST_CASE("constructor_with_time") {
    auto time = time_from_string("2025-10-12 14:30:00");
    auto client = FixedTimeClient(time);
    auto result = client.get_time();
    REQUIRE(result == time);
  }

  TEST_CASE("set_and_get_time") {
    auto client = FixedTimeClient();
    auto time = time_from_string("2025-10-12 10:15:00");
    client.set(time);
    auto result = client.get_time();
    REQUIRE(result == time);
  }

  TEST_CASE("update_time") {
    auto initial = time_from_string("2025-01-01 00:00:00");
    auto client = FixedTimeClient(initial);
    auto updated = time_from_string("2025-12-31 23:59:00");
    client.set(updated);
    auto result = client.get_time();
    REQUIRE(result == updated);
  }

  TEST_CASE("get_time_after_close_throws") {
    auto time = time_from_string("2025-10-12 12:00:00");
    auto client = FixedTimeClient(time);
    client.close();
    REQUIRE_THROWS(client.get_time());
  }

  TEST_CASE("set_time_after_close_throws") {
    auto time = time_from_string("2025-10-12 12:00:00");
    auto client = FixedTimeClient(time);
    client.close();
    auto new_time = time_from_string("2025-10-13 12:00:00");
    REQUIRE_THROWS(client.set(new_time));
  }
}
