#ifndef BEAM_SERVICE_LOCATOR_DATA_STORE_TEST_SUITE_HPP
#define BEAM_SERVICE_LOCATOR_DATA_STORE_TEST_SUITE_HPP
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"

namespace Beam::Tests {
  TEST_CASE_TEMPLATE_DEFINE(
      "ServiceLocatorDataStore", T, ServiceLocatorDataStoreTestSuite) {
    using namespace Beam;
    using namespace boost;
    using namespace boost::posix_time;
    auto data_store = T()();
    data_store.make_directory(
      "*", DirectoryEntry::make_directory(static_cast<unsigned int>(-1), "*"));

    SUBCASE("store_and_load_account") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account =
        data_store.make_account("test_account", "hashed_password",
          DirectoryEntry::STAR_DIRECTORY, time);
      auto loaded = data_store.load_directory_entry(account.m_id);
      REQUIRE(loaded.m_id == account.m_id);
      REQUIRE(loaded.m_name == account.m_name);
      REQUIRE(loaded.m_type == DirectoryEntry::Type::ACCOUNT);
    }

    SUBCASE("store_and_load_directory") {
      auto directory =
        data_store.make_directory("test_dir", DirectoryEntry::STAR_DIRECTORY);
      auto loaded = data_store.load_directory_entry(directory.m_id);
      REQUIRE(loaded.m_id == directory.m_id);
      REQUIRE(loaded.m_name == directory.m_name);
      REQUIRE(loaded.m_type == DirectoryEntry::Type::DIRECTORY);
    }

    SUBCASE("load_account_by_name") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account = data_store.make_account(
        "named_account", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto loaded = data_store.load_account("named_account");
      REQUIRE(loaded.m_id == account.m_id);
      REQUIRE(loaded.m_name == "named_account");
    }

    SUBCASE("load_nonexistent_account") {
      REQUIRE_THROWS(data_store.load_account("nonexistent"));
    }

    SUBCASE("load_nonexistent_directory_entry") {
      REQUIRE_THROWS(data_store.load_directory_entry(999));
    }

    SUBCASE("make_account") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account = data_store.make_account(
        "new_account", "password", DirectoryEntry::STAR_DIRECTORY, time);
      REQUIRE(account.m_name == "new_account");
      REQUIRE(account.m_type == DirectoryEntry::Type::ACCOUNT);
      auto loaded = data_store.load_account("new_account");
      REQUIRE(loaded == account);
    }

    SUBCASE("make_duplicate_account") {
      auto time = time_from_string("2024-01-01 12:00:00");
      data_store.make_account(
        "duplicate", "password", DirectoryEntry::STAR_DIRECTORY, time);
      REQUIRE_THROWS(data_store.make_account(
        "duplicate", "password", DirectoryEntry::STAR_DIRECTORY, time));
    }

    SUBCASE("make_directory") {
      auto directory =
        data_store.make_directory("new_dir", DirectoryEntry::STAR_DIRECTORY);
      REQUIRE(directory.m_name == "new_dir");
      REQUIRE(directory.m_type == DirectoryEntry::Type::DIRECTORY);
      auto loaded = data_store.load_directory_entry(directory.m_id);
      REQUIRE(loaded == directory);
    }

    SUBCASE("associate_and_load_parents") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto parent =
        data_store.make_directory("parent", DirectoryEntry::STAR_DIRECTORY);
      auto child = data_store.make_account(
        "child", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto associated = data_store.associate(child, parent);
      REQUIRE(associated);
      auto parents = data_store.load_parents(child);
      REQUIRE(parents.size() == 2);
      REQUIRE(std::ranges::contains(parents, parent));
      REQUIRE(std::ranges::contains(parents, DirectoryEntry::STAR_DIRECTORY));
    }

    SUBCASE("associate_duplicate") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto parent =
        data_store.make_directory("parent2", DirectoryEntry::STAR_DIRECTORY);
      auto child = data_store.make_account(
        "child2", "password", DirectoryEntry::STAR_DIRECTORY, time);
      data_store.associate(child, parent);
      auto associated = data_store.associate(child, parent);
      REQUIRE(!associated);
    }

    SUBCASE("associate_with_invalid_parent") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto child = data_store.make_account(
        "orphan", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto invalid_parent = DirectoryEntry::make_directory(999, "missing");
      REQUIRE_THROWS(data_store.associate(child, invalid_parent));
    }

    SUBCASE("load_children") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto parent =
        data_store.make_directory("parent3", DirectoryEntry::STAR_DIRECTORY);
      auto child1 = data_store.make_account(
        "child3", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto child2 = data_store.make_account(
        "child4", "password", DirectoryEntry::STAR_DIRECTORY, time);
      data_store.associate(child1, parent);
      data_store.associate(child2, parent);
      auto children = data_store.load_children(parent);
      REQUIRE(children.size() == 2);
      REQUIRE(
        std::find(children.begin(), children.end(), child1) != children.end());
      REQUIRE(
        std::find(children.begin(), children.end(), child2) != children.end());
    }

    SUBCASE("load_empty_children") {
      auto parent = data_store.make_directory(
        "empty_parent", DirectoryEntry::STAR_DIRECTORY);
      auto children = data_store.load_children(parent);
      REQUIRE(children.empty());
    }

    SUBCASE("detach") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto parent =
        data_store.make_directory("parent4", DirectoryEntry::STAR_DIRECTORY);
      auto child = data_store.make_account("child5", "password", parent, time);
      auto detached = data_store.detach(child, parent);
      REQUIRE(detached);
      auto parents = data_store.load_parents(child);
      REQUIRE(parents.empty());
    }

    SUBCASE("detach_nonexistent") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto parent =
        data_store.make_directory("parent5", DirectoryEntry::STAR_DIRECTORY);
      auto child = data_store.make_account(
        "child6", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto detached = data_store.detach(child, parent);
      REQUIRE(!detached);
    }

    SUBCASE("remove_empty_entry") {
      auto entry =
        data_store.make_directory("to_remove", DirectoryEntry::STAR_DIRECTORY);
      data_store.remove(entry);
      REQUIRE_THROWS(data_store.load_directory_entry(entry.m_id));
    }

    SUBCASE("remove_nonempty_entry") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto parent =
        data_store.make_directory("parent6", DirectoryEntry::STAR_DIRECTORY);
      auto child = data_store.make_account("child7", "password", parent, time);
      REQUIRE_THROWS(data_store.remove(parent));
    }

    SUBCASE("load_and_set_password") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account = data_store.make_account(
        "pwd_account", "initial_password", DirectoryEntry::STAR_DIRECTORY, time);
      auto password = data_store.load_password(account);
      REQUIRE(validate_password(account, "initial_password", password));
      data_store.set_password(account, hash_password(account, "new_password"));
      password = data_store.load_password(account);
      REQUIRE(validate_password(account, "new_password", password));
    }

    SUBCASE("load_and_set_permissions") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto source = data_store.make_account(
        "source", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto target = data_store.make_account(
        "target", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto initial = data_store.load_permissions(source, target);
      REQUIRE(initial == Permission::NONE);
      auto permissions = Permission::READ;
      data_store.set_permissions(source, target, permissions);
      auto loaded = data_store.load_permissions(source, target);
      REQUIRE(loaded == permissions);
    }

    SUBCASE("load_all_permissions") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto source = data_store.make_account(
        "source2", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto target1 = data_store.make_account(
        "target1", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto target2 = data_store.make_account(
        "target2", "password", DirectoryEntry::STAR_DIRECTORY, time);
      data_store.set_permissions(source, target1, Permission::READ);
      data_store.set_permissions(source, target2, Permission::ADMINISTRATE);
      auto all = data_store.load_all_permissions(source);
      REQUIRE(all.size() == 2);
      auto contains_target1 = false;
      auto contains_target2 = false;
      for(auto& [target, permissions] : all) {
        if(target == target1 && permissions == Permission::READ) {
          contains_target1 = true;
        }
        if(target == target2 && permissions == Permission::ADMINISTRATE) {
          contains_target2 = true;
        }
      }
      REQUIRE(contains_target1);
      REQUIRE(contains_target2);
    }

    SUBCASE("clear_permissions") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto source = data_store.make_account(
        "source3", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto target = data_store.make_account(
        "target3", "password", DirectoryEntry::STAR_DIRECTORY, time);
      data_store.set_permissions(source, target, Permission::READ);
      data_store.set_permissions(source, target, Permissions());
      auto loaded = data_store.load_permissions(source, target);
      REQUIRE(loaded == Permission::NONE);
    }

    SUBCASE("load_registration_time") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account = data_store.make_account(
        "reg_account", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto loaded_time = data_store.load_registration_time(account);
      REQUIRE(loaded_time == time);
    }

    SUBCASE("load_and_store_last_login_time") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account = data_store.make_account(
        "login_account", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto loaded = data_store.load_last_login_time(account);
      REQUIRE(loaded == neg_infin);
      auto new_time = time_from_string("2024-01-01 13:00:00");
      data_store.store_last_login_time(account, new_time);
      loaded = data_store.load_last_login_time(account);
      REQUIRE(loaded == new_time);
    }

    SUBCASE("rename_account") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account = data_store.make_account(
        "old_name", "password", DirectoryEntry::STAR_DIRECTORY, time);
      data_store.rename(account, "new_name");
      auto loaded = data_store.load_account("new_name");
      REQUIRE(loaded.m_id == account.m_id);
      REQUIRE_THROWS(data_store.load_account("old_name"));
    }

    SUBCASE("rename_directory") {
      auto directory =
        data_store.make_directory("old_dir", DirectoryEntry::STAR_DIRECTORY);
      data_store.rename(directory, "new_dir_name");
      auto loaded = data_store.load_directory_entry(directory.m_id);
      REQUIRE(loaded.m_name == "new_dir_name");
    }

    SUBCASE("load_all_accounts") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account1 = data_store.make_account(
        "account1", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto account2 = data_store.make_account(
        "account2", "password", DirectoryEntry::STAR_DIRECTORY, time);
      auto accounts = data_store.load_all_accounts();
      REQUIRE(accounts.size() >= 2);
      REQUIRE(std::find(accounts.begin(), accounts.end(), account1) !=
        accounts.end());
      REQUIRE(std::find(accounts.begin(), accounts.end(), account2) !=
        accounts.end());
    }

    SUBCASE("load_all_directories") {
      auto directory1 =
        data_store.make_directory("dir1", DirectoryEntry::STAR_DIRECTORY);
      auto directory2 =
        data_store.make_directory("dir2", DirectoryEntry::STAR_DIRECTORY);
      auto directories = data_store.load_all_directories();
      REQUIRE(directories.size() >= 3);
      REQUIRE(std::find(directories.begin(), directories.end(),
        DirectoryEntry::STAR_DIRECTORY) != directories.end());
      REQUIRE(std::find(directories.begin(), directories.end(), directory1) !=
        directories.end());
      REQUIRE(std::find(directories.begin(), directories.end(), directory2) !=
        directories.end());
    }

    SUBCASE("transaction_isolation") {
      auto time = time_from_string("2024-01-01 12:00:00");
      auto account_id = data_store.with_transaction([&] {
        auto account = data_store.make_account(
          "transactional", "password", DirectoryEntry::STAR_DIRECTORY, time);
        return account.m_id;
      });
      REQUIRE(account_id != 0);
      auto loaded = data_store.load_directory_entry(account_id);
      REQUIRE(loaded.m_name == "transactional");
    }

    SUBCASE("close") {
      data_store.close();
      REQUIRE_THROWS(data_store.load_all_accounts());
    }

    SUBCASE("multiple_close") {
      data_store.close();
      data_store.close();
      REQUIRE_THROWS(data_store.load_all_accounts());
    }
  }
}

#endif
