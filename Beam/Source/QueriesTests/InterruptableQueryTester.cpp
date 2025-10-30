#include <doctest/doctest.h>
#include "Beam/Queries/InterruptableQuery.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("InterruptableQuery") {
  TEST_CASE("default_constructor") {
    auto query = InterruptableQuery();
    REQUIRE(query.get_interruption_policy() == InterruptionPolicy::BREAK_QUERY);
  }

  TEST_CASE("interruption_policy_constructor") {
    auto break_query = InterruptableQuery(InterruptionPolicy::BREAK_QUERY);
    REQUIRE(
      break_query.get_interruption_policy() == InterruptionPolicy::BREAK_QUERY);
    auto recover_query = InterruptableQuery(InterruptionPolicy::RECOVER_DATA);
    REQUIRE(recover_query.get_interruption_policy() ==
      InterruptionPolicy::RECOVER_DATA);
    auto continue_query =
      InterruptableQuery(InterruptionPolicy::IGNORE_CONTINUE);
    REQUIRE(continue_query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("set_interruption_policy") {
    auto query = InterruptableQuery();
    REQUIRE(
      query.get_interruption_policy() != InterruptionPolicy::IGNORE_CONTINUE);
    query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
    REQUIRE(
      query.get_interruption_policy() == InterruptionPolicy::IGNORE_CONTINUE);
    query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
    REQUIRE(
      query.get_interruption_policy() == InterruptionPolicy::RECOVER_DATA);
    query.set_interruption_policy(InterruptionPolicy::BREAK_QUERY);
    REQUIRE(query.get_interruption_policy() == InterruptionPolicy::BREAK_QUERY);
  }

  TEST_CASE("stream") {
    auto query = InterruptableQuery(InterruptionPolicy::BREAK_QUERY);
    REQUIRE(to_string(query) == "BREAK_QUERY");
    test_round_trip_shuttle(query);
  }
}
