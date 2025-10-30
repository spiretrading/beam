#include <unordered_set>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("DirectoryEntry") {
  TEST_CASE("make_account") {
    auto entry = DirectoryEntry::make_account(42, "test_account");
    REQUIRE(entry.m_type == DirectoryEntry::Type::ACCOUNT);
    REQUIRE(entry.m_id == 42);
    REQUIRE(entry.m_name == "test_account");
  }

  TEST_CASE("make_account_without_name") {
    auto account = DirectoryEntry::make_account(75);
    REQUIRE(account.m_type == DirectoryEntry::Type::ACCOUNT);
    REQUIRE(account.m_id == 75);
    REQUIRE(account.m_name.empty());
  }

  TEST_CASE("make_directory") {
    auto entry = DirectoryEntry::make_directory(100, "test_directory");
    REQUIRE(entry.m_type == DirectoryEntry::Type::DIRECTORY);
    REQUIRE(entry.m_id == 100);
    REQUIRE(entry.m_name == "test_directory");
  }

  TEST_CASE("make_directory_without_name") {
    auto directory = DirectoryEntry::make_directory(300);
    REQUIRE(directory.m_type == DirectoryEntry::Type::DIRECTORY);
    REQUIRE(directory.m_id == 300);
    REQUIRE(directory.m_name.empty());
  }

  TEST_CASE("get_root_account") {
    auto& root = DirectoryEntry::ROOT_ACCOUNT;
    REQUIRE(root.m_type == DirectoryEntry::Type::ACCOUNT);
    REQUIRE(root.m_id == 1);
    REQUIRE(root.m_name == "root");
  }

  TEST_CASE("get_star_directory") {
    auto& star = DirectoryEntry::STAR_DIRECTORY;
    REQUIRE(star.m_type == DirectoryEntry::Type::DIRECTORY);
    REQUIRE(star.m_id == 0);
    REQUIRE(star.m_name == "*");
  }

  TEST_CASE("name_comparator_by_name") {
    auto entry1 = DirectoryEntry::make_account(1, "alice");
    auto entry2 = DirectoryEntry::make_account(2, "bob");
    REQUIRE(DirectoryEntry::name_comparator(entry1, entry2));
    REQUIRE_FALSE(DirectoryEntry::name_comparator(entry2, entry1));
  }

  TEST_CASE("name_comparator_same_name_different_id") {
    auto entry1 = DirectoryEntry::make_account(1, "alice");
    auto entry2 = DirectoryEntry::make_account(2, "alice");
    REQUIRE(DirectoryEntry::name_comparator(entry1, entry2));
    REQUIRE_FALSE(DirectoryEntry::name_comparator(entry2, entry1));
  }

  TEST_CASE("name_comparator_same_entry") {
    auto entry = DirectoryEntry::make_account(1, "alice");
    REQUIRE_FALSE(DirectoryEntry::name_comparator(entry, entry));
  }

  TEST_CASE("less_than_operator") {
    auto entry1 = DirectoryEntry::make_account(10, "first");
    auto entry2 = DirectoryEntry::make_account(20, "second");
    REQUIRE(entry1 < entry2);
    REQUIRE_FALSE(entry2 < entry1);
    REQUIRE_FALSE(entry1 < entry1);
  }

  TEST_CASE("equality_same_id_and_type") {
    auto entry1 = DirectoryEntry::make_account(42, "alice");
    auto entry2 = DirectoryEntry::make_account(42, "alice");
    REQUIRE(entry1 == entry2);
  }

  TEST_CASE("equality_same_id_different_names") {
    auto entry1 = DirectoryEntry::make_account(42, "alice");
    auto entry2 = DirectoryEntry::make_account(42, "bob");
    REQUIRE(entry1 == entry2);
  }

  TEST_CASE("equality_different_ids") {
    auto entry1 = DirectoryEntry::make_account(1, "alice");
    auto entry2 = DirectoryEntry::make_account(2, "alice");
    REQUIRE_FALSE(entry1 == entry2);
  }

  TEST_CASE("equality_same_id_different_types") {
    auto account = DirectoryEntry::make_account(42, "test");
    auto directory = DirectoryEntry::make_directory(42, "test");
    REQUIRE_FALSE(account == directory);
  }

  TEST_CASE("equality_default_entries") {
    auto entry1 = DirectoryEntry();
    auto entry2 = DirectoryEntry();
    REQUIRE(entry1 == entry2);
  }

  TEST_CASE("hash_value_same_id") {
    auto entry1 = DirectoryEntry::make_account(100, "alice");
    auto entry2 = DirectoryEntry::make_account(100, "bob");
    REQUIRE(hash_value(entry1) == hash_value(entry2));
  }

  TEST_CASE("stream") {
    auto account = DirectoryEntry::make_account(42, "alice");
    REQUIRE(to_string(account) == "(ACCOUNT 42 alice)");
    test_round_trip_shuttle(account);
  }

  TEST_CASE("stream_none") {
    auto entry = DirectoryEntry(DirectoryEntry::Type::NONE, 0, "test");
    REQUIRE(to_string(entry) == "NONE");
  }
}
