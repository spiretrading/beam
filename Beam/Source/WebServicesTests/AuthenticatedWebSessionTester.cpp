#include <doctest/doctest.h>
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/WebServices/AuthenticatedWebSession.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("AuthenticatedWebSession") {
  TEST_CASE("construct") {
    auto session = AuthenticatedWebSession("session123");
    REQUIRE(session.get_id() == "session123");
    REQUIRE(!session.is_expired());
    REQUIRE(!session.is_logged_in());
    auto account = session.get_account();
    REQUIRE(account.m_id == static_cast<unsigned int>(-1));
  }

  TEST_CASE("set_account") {
    auto session = AuthenticatedWebSession("auth_session");
    REQUIRE(!session.is_logged_in());
    auto account = DirectoryEntry::make_account(100, "test_user");
    session.set_account(account);
    REQUIRE(session.is_logged_in());
    auto retrieved = session.get_account();
    REQUIRE(retrieved.m_id == 100);
    REQUIRE(retrieved.m_name == "test_user");
    REQUIRE(retrieved.m_type == DirectoryEntry::Type::ACCOUNT);
  }

  TEST_CASE("reset_account") {
    auto session = AuthenticatedWebSession("logout_session");
    auto account = DirectoryEntry::make_account(200, "logout_user");
    session.set_account(account);
    REQUIRE(session.is_logged_in());
    session.reset_account();
    REQUIRE(!session.is_logged_in());
    auto retrieved = session.get_account();
    REQUIRE(retrieved.m_id == static_cast<unsigned int>(-1));
  }

  TEST_CASE("shuttle") {
    auto session = AuthenticatedWebSession("serialization_session");
    auto account = DirectoryEntry::make_account(999, "serialized_user");
    session.set_account(account);
    session.set_expired();
    test_round_trip_shuttle(session, [&] (auto&& received) {
      REQUIRE(received.get_id() == session.get_id());
      REQUIRE(received.is_expired() == session.is_expired());
      REQUIRE(received.is_logged_in() == session.is_logged_in());
      auto received_account = received.get_account();
      REQUIRE(received_account.m_id == account.m_id);
      REQUIRE(received_account.m_name == account.m_name);
      REQUIRE(received_account.m_type == account.m_type);
    });
  }
}
