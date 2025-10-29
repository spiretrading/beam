#include <sstream>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/AccountUpdate.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("AccountUpdate") {
  TEST_CASE("make_add") {
    auto account = DirectoryEntry::make_account(42, "test_user");
    auto update = AccountUpdate::add(account);
    REQUIRE(update.m_account == account);
    REQUIRE(update.m_type == AccountUpdate::Type::ADDED);
  }

  TEST_CASE("make_remove") {
    auto account = DirectoryEntry::make_account(100, "deleted_user");
    auto update = AccountUpdate::remove(account);
    REQUIRE(update.m_account == account);
    REQUIRE(update.m_type == AccountUpdate::Type::DELETED);
  }

  TEST_CASE("equality_same_updates") {
    auto account = DirectoryEntry::make_account(1, "user");
    auto update1 = AccountUpdate::add(account);
    auto update2 = AccountUpdate::add(account);
    REQUIRE(update1 == update2);
  }

  TEST_CASE("equality_different_accounts") {
    auto account1 = DirectoryEntry::make_account(1, "user1");
    auto account2 = DirectoryEntry::make_account(2, "user2");
    auto update1 = AccountUpdate::add(account1);
    auto update2 = AccountUpdate::add(account2);
    REQUIRE_FALSE(update1 == update2);
  }

  TEST_CASE("equality_different_types") {
    auto account = DirectoryEntry::make_account(1, "user");
    auto update1 = AccountUpdate::add(account);
    auto update2 = AccountUpdate::remove(account);
    REQUIRE_FALSE(update1 == update2);
  }

  TEST_CASE("stream_type") {
    auto ss = std::stringstream();
    ss << AccountUpdate::Type::ADDED;
    REQUIRE(ss.str() == "ADDED");
    ss.str("");
    ss << AccountUpdate::Type::DELETED;
    REQUIRE(ss.str() == "DELETED");
  }

  TEST_CASE("stream") {
    auto account = DirectoryEntry::make_account(42, "test_user");
    auto update = AccountUpdate::add(account);
    auto ss = std::stringstream();
    ss << update;
    REQUIRE(ss.str() == "((ACCOUNT 42 test_user) ADDED)");
    test_round_trip_shuttle(update);
  }
}
