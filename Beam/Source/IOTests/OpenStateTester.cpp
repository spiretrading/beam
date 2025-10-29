#include <doctest/doctest.h>
#include "Beam/IO/OpenState.hpp"

using namespace Beam;

TEST_SUITE("OpenState") {
  TEST_CASE("initial_state_is_open") {
    auto state = OpenState();
    REQUIRE(state.is_open());
    REQUIRE(!state.is_closing());
    REQUIRE(!state.is_closed());
    REQUIRE_NOTHROW(state.ensure_open());
    state.close();
  }

  TEST_CASE("set_closing") {
    auto state = OpenState();
    auto prior = state.set_closing();
    REQUIRE(!prior);
    REQUIRE(!state.is_open());
    REQUIRE(state.is_closing());
    REQUIRE(!state.is_closed());
    state.close();
    REQUIRE(!state.is_open());
    REQUIRE(!state.is_closing());
    REQUIRE(state.is_closed());
    REQUIRE_THROWS_AS(state.ensure_open(), EndOfFileException);
  }

  TEST_CASE("set_closing_redundant") {
    auto state = OpenState();
    state.close();
    auto prior = state.set_closing();
    REQUIRE(prior);
    REQUIRE(state.is_closed());
  }
}
