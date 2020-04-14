#include <doctest/doctest.h>
#include "Beam/Queries/InterruptableQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("InterruptableQuery") {
  TEST_CASE("default_constructor") {
    auto query = InterruptableQuery();
    REQUIRE(query.GetInterruptionPolicy() == InterruptionPolicy::BREAK_QUERY);
  }

  TEST_CASE("interruption_policy_constructor") {
    auto breakQuery = InterruptableQuery(InterruptionPolicy::BREAK_QUERY);
    REQUIRE(breakQuery.GetInterruptionPolicy() ==
      InterruptionPolicy::BREAK_QUERY);
    auto recoverQuery = InterruptableQuery(InterruptionPolicy::RECOVER_DATA);
    REQUIRE(recoverQuery.GetInterruptionPolicy() ==
      InterruptionPolicy::RECOVER_DATA);
    auto continueQuery = InterruptableQuery(
      InterruptionPolicy::IGNORE_CONTINUE);
    REQUIRE(continueQuery.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("set_interruption_policy") {
    auto query = InterruptableQuery();
    REQUIRE(query.GetInterruptionPolicy() !=
      InterruptionPolicy::IGNORE_CONTINUE);
    query.SetInterruptionPolicy(InterruptionPolicy::IGNORE_CONTINUE);
    REQUIRE(query.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
    query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
    REQUIRE(query.GetInterruptionPolicy() ==
      InterruptionPolicy::RECOVER_DATA);
    query.SetInterruptionPolicy(InterruptionPolicy::BREAK_QUERY);
    REQUIRE(query.GetInterruptionPolicy() ==
      InterruptionPolicy::BREAK_QUERY);
  }
}
