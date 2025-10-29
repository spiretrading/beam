#include <doctest/doctest.h>
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/WebServices/WebSession.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("WebSession") {
  TEST_CASE("construct") {
    auto session = WebSession("session123");
    REQUIRE(session.get_id() == "session123");
    REQUIRE(!session.is_expired());
    auto empty = WebSession("");
    REQUIRE(empty.get_id() == "");
    REQUIRE(!empty.is_expired());
  }

  TEST_CASE("set_expired") {
    auto session = WebSession("expire_me");
    REQUIRE(!session.is_expired());
    session.set_expired();
    REQUIRE(session.is_expired());
    session.set_expired();
    REQUIRE(session.is_expired());
  }

  TEST_CASE("shuttle") {
    auto session = WebSession(
      "very_long_session_id_with_many_characters_to_test_serialization");
    session.set_expired();
    test_round_trip_shuttle(session, [&] (auto&& received) {
      REQUIRE(received.get_id() == session.get_id());
      REQUIRE(received.is_expired() == session.is_expired());
    });
  }
}
