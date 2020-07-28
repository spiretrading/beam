#ifndef BEAM_SERVICE_LOCATOR_SQL_DEFINITIONS_HPP
#define BEAM_SERVICE_LOCATOR_SQL_DEFINITIONS_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <Viper/Row.hpp>
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Returns a row representing the settings table. */
  inline const auto& GetSettingsRow() {
    static const auto ROW = Viper::Row<std::uint32_t>("next_entry_id");
    return ROW;
  }

  /** Returns a row representing a DirectoryEntry. */
  inline const auto& GetDirectoryEntryRow() {
    static const auto ROW = Viper::Row<DirectoryEntry>().
      add_column("id", &DirectoryEntry::m_id).
      add_column("name", Viper::varchar(100), &DirectoryEntry::m_name);
    return ROW;
  }

  /** Stores an SQL row representing an account. */
  struct AccountsRow {

    /** The account's DirectoryEntry. */
    DirectoryEntry m_entry;

    /** The password. */
    std::string m_password;

    /** When the account was registered. */
    boost::posix_time::ptime m_registrationTime;

    /** When the account last logged in. */
    boost::posix_time::ptime m_lastLogin;
  };

  /** Returns a row representing an account. */
  inline const auto& GetAccountsRow() {
    static const auto ROW = Viper::Row<AccountsRow>().extend(
      GetDirectoryEntryRow(), &AccountsRow::m_entry).
      add_column("password", Viper::varchar(100), &AccountsRow::m_password).
      add_column("registration_time", &AccountsRow::m_registrationTime).
      add_column("last_login_time", &AccountsRow::m_lastLogin).
      set_primary_key("id");
    return ROW;
  }

  /** Returns a row representing a directory. */
  inline const auto& GetDirectoriesRow() {
    static const auto ROW = GetDirectoryEntryRow().set_primary_key("id");
    return ROW;
  }

  /** Stores an SQL parents row. */
  struct ParentsRow {

    /** The entry to lookup. */
    unsigned int m_entry;

    /** The id of the entry's parent. */
    unsigned int m_parent;
  };

  /** Returns a row representing a parent entry. */
  inline const auto& GetParentsRow() {
    static const auto ROW = Viper::Row<ParentsRow>().
      add_column("entry", &ParentsRow::m_entry).
      add_column("parent", &ParentsRow::m_parent);
    return ROW;
  }

  /** Stores an SQL childrens row. */
  struct ChildrenRow {

    /** The entry to lookup. */
    unsigned int m_entry;

    /** The id of the entry's child. */
    unsigned int m_child;
  };

  /** Returns a row representing a child entry. */
  inline const auto& GetChildrenRow() {
    static const auto ROW = Viper::Row<ChildrenRow>().
      add_column("entry", &ChildrenRow::m_entry).
      add_column("child", &ChildrenRow::m_child);
    return ROW;
  }

  /** Stores an SQL permissions row. */
  struct PermissionsRow {

    /** The id of the source. */
    unsigned int m_source;

    /** The id of the target. */
    unsigned int m_target;

    /** The permission that <i>source</i> has over the <i>target</i>. */
    unsigned int m_permission;
  };

  /** Returns a row representing a permission. */
  inline const auto& GetPermissionsRow() {
    static const auto ROW = Viper::Row<PermissionsRow>().
      add_column("source", &PermissionsRow::m_source).
      add_column("target", &PermissionsRow::m_target).
      add_column("permission", &PermissionsRow::m_permission).
      set_primary_key({"source", "target"});
    return ROW;
  }
}

#endif
