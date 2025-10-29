#include <doctest/doctest.h>
#include <thread>
#include <vector>
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"

using namespace Beam;

TEST_SUITE("AuthenticatedSession") {
  TEST_CASE("constructor") {
    auto session = AuthenticatedSession();
    REQUIRE_FALSE(session.is_logged_in());
    auto account = session.get_account();
    REQUIRE(account.m_id == static_cast<unsigned int>(-1));
    REQUIRE(account.m_type == DirectoryEntry::Type::NONE);
  }

  TEST_CASE("set_account") {
    auto session = AuthenticatedSession();
    auto account = DirectoryEntry::make_account(42, "alice");
    session.set_account(account);
    REQUIRE(session.is_logged_in());
    REQUIRE(session.get_account() == account);
  }

  TEST_CASE("reset_account") {
    auto session = AuthenticatedSession();
    auto account = DirectoryEntry::make_account(100, "bob");
    session.set_account(account);
    REQUIRE(session.is_logged_in());
    session.reset_account();
    REQUIRE_FALSE(session.is_logged_in());
    REQUIRE(session.get_account().m_id == static_cast<unsigned int>(-1));
  }

  TEST_CASE("copy_constructor_not_logged_in") {
    auto original = AuthenticatedSession();
    auto copy = original;
    REQUIRE_FALSE(copy.is_logged_in());
    REQUIRE(copy.get_account().m_id == static_cast<unsigned int>(-1));
  }

  TEST_CASE("copy_constructor_logged_in") {
    auto original = AuthenticatedSession();
    auto account = DirectoryEntry::make_account(50, "charlie");
    original.set_account(account);
    auto copy = original;
    REQUIRE(copy.is_logged_in());
    REQUIRE(copy.get_account() == account);
  }

  TEST_CASE("move_constructor_not_logged_in") {
    auto original = AuthenticatedSession();
    auto moved = std::move(original);
    REQUIRE_FALSE(moved.is_logged_in());
    REQUIRE(moved.get_account().m_id == static_cast<unsigned int>(-1));
  }

  TEST_CASE("move_constructor_logged_in") {
    auto original = AuthenticatedSession();
    auto account = DirectoryEntry::make_account(75, "david");
    original.set_account(account);
    auto moved = std::move(original);
    REQUIRE(moved.is_logged_in());
    REQUIRE(moved.get_account() == account);
  }

  TEST_CASE("copy_assignment_not_logged_in") {
    auto session1 = AuthenticatedSession();
    auto session2 = AuthenticatedSession();
    session1 = session2;
    REQUIRE_FALSE(session1.is_logged_in());
  }

  TEST_CASE("copy_assignment_logged_in") {
    auto session1 = AuthenticatedSession();
    auto session2 = AuthenticatedSession();
    auto account = DirectoryEntry::make_account(200, "eve");
    session2.set_account(account);
    session1 = session2;
    REQUIRE(session1.is_logged_in());
    REQUIRE(session1.get_account() == account);
    REQUIRE(session2.is_logged_in());
  }

  TEST_CASE("move_assignment_not_logged_in") {
    auto session1 = AuthenticatedSession();
    auto session2 = AuthenticatedSession();
    session1 = std::move(session2);
    REQUIRE_FALSE(session1.is_logged_in());
  }

  TEST_CASE("move_assignment_logged_in") {
    auto session1 = AuthenticatedSession();
    auto session2 = AuthenticatedSession();
    auto account = DirectoryEntry::make_account(300, "frank");
    session2.set_account(account);
    session1 = std::move(session2);
    REQUIRE(session1.is_logged_in());
    REQUIRE(session1.get_account() == account);
  }
}
