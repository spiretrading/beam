#ifndef BEAM_SQL_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_SQL_SERVICE_LOCATOR_DATA_STORE_HPP
#include <boost/throw_exception.hpp>
#include <Viper/Viper.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/ServiceLocator/SqlDefinitions.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /**
   * Implements the ServiceLocatorDataStore using SQL.
   * @tparam C The type of SQL connection to use.
   */
  template<typename C>
  class SqlServiceLocatorDataStore {
    public:

      /** The type of SQL connection to use. */
      using Connection = C;

      /**
       * Constructs an SqlServiceLocatorDataStore.
       * @param connection The SQL connection to use.
       */
      explicit SqlServiceLocatorDataStore(
        std::unique_ptr<Connection> connection);

      ~SqlServiceLocatorDataStore();

      std::vector<DirectoryEntry> load_parents(const DirectoryEntry& entry);
      std::vector<DirectoryEntry> load_children(
        const DirectoryEntry& directory);
      DirectoryEntry load_directory_entry(unsigned int id);
      std::vector<DirectoryEntry> load_all_accounts();
      std::vector<DirectoryEntry> load_all_directories();
      DirectoryEntry load_account(const std::string& name);
      DirectoryEntry make_account(const std::string& name,
        const std::string& password, const DirectoryEntry& parent,
        boost::posix_time::ptime registration_time);
      DirectoryEntry make_directory(
        const std::string& name, const DirectoryEntry& parent);
      void remove(const DirectoryEntry& entry);
      bool associate(const DirectoryEntry& entry, const DirectoryEntry& parent);
      bool detach(const DirectoryEntry& entry, const DirectoryEntry& parent);
      std::string load_password(const DirectoryEntry& account);
      void set_password(
        const DirectoryEntry& account, const std::string& password);
      Permissions load_permissions(
        const DirectoryEntry& source, const DirectoryEntry& target);
      std::vector<std::tuple<DirectoryEntry, Permissions>> load_all_permissions(
        const DirectoryEntry& account);
      void set_permissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions);
      boost::posix_time::ptime load_registration_time(
        const DirectoryEntry& account);
      boost::posix_time::ptime load_last_login_time(
        const DirectoryEntry& account);
      void store_last_login_time(
        const DirectoryEntry& account, boost::posix_time::ptime login_time);
      void rename(const DirectoryEntry& entry, const std::string& name);
      DirectoryEntry validate(const DirectoryEntry& entry);
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);
      void close();

    private:
      mutable Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      unsigned int m_next_entry_id;
      OpenState m_open_state;

      unsigned int load_next_entry_id();
  };

  template<typename C>
  SqlServiceLocatorDataStore<C>::SqlServiceLocatorDataStore(
      std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {
    try {
      m_connection->open();
      if(!m_connection->has_table("settings")) {
        m_connection->execute(Viper::create(get_settings_row(), "settings"));
        auto first_entry_id = 0UL;
        m_connection->execute(
          Viper::insert(get_settings_row(), "settings", &first_entry_id));
      }
      m_connection->execute(
        Viper::create_if_not_exists(get_accounts_row(), "accounts"));
      m_connection->execute(
        Viper::create_if_not_exists(get_directories_row(), "directories"));
      m_connection->execute(
        Viper::create_if_not_exists(get_parents_row(), "parents"));
      m_connection->execute(
        Viper::create_if_not_exists(get_children_row(), "children"));
      m_connection->execute(
        Viper::create_if_not_exists(get_permissions_row(), "permissions"));
      m_connection->execute(
        Viper::select(get_settings_row(), "settings", &m_next_entry_id));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlServiceLocatorDataStore<C>::~SqlServiceLocatorDataStore() {
    close();
  }

  template<typename C>
  std::vector<DirectoryEntry>
      SqlServiceLocatorDataStore<C>::load_parents(const DirectoryEntry& entry) {
    auto parent_ids = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("parent"),
        "parents", Viper::sym("entry") == entry.m_id,
        std::back_inserter(parent_ids)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    auto parents = std::vector<DirectoryEntry>();
    for(auto& parent_id : parent_ids) {
      parents.push_back(load_directory_entry(parent_id));
    }
    return parents;
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::load_children(
      const DirectoryEntry& entry) {
    auto child_ids = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("child"),
        "children", Viper::sym("entry") == entry.m_id,
        std::back_inserter(child_ids)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    auto children = std::vector<DirectoryEntry>();
    for(auto& child_id : child_ids) {
      children.push_back(load_directory_entry(child_id));
    }
    return children;
  }

  template<typename C>
  DirectoryEntry
      SqlServiceLocatorDataStore<C>::load_directory_entry(unsigned int id) {
    auto entry = std::optional<DirectoryEntry>();
    try {
      m_connection->execute(Viper::select(
        get_directory_entry_row(), "accounts", Viper::sym("id") == id, &entry));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    if(entry) {
      entry->m_type = DirectoryEntry::Type::ACCOUNT;
      return std::move(*entry);
    }
    try {
      m_connection->execute(Viper::select(get_directory_entry_row(),
        "directories", Viper::sym("id") == id, &entry));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    if(entry) {
      entry->m_type = DirectoryEntry::Type::DIRECTORY;
      return std::move(*entry);
    }
    boost::throw_with_location(
      ServiceLocatorDataStoreException("Directory entry not found."));
  }

  template<typename C>
  std::vector<DirectoryEntry>
      SqlServiceLocatorDataStore<C>::load_all_accounts() {
    auto account_ids = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("id"),
        "accounts", std::back_inserter(account_ids)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    auto accounts = std::vector<DirectoryEntry>();
    for(auto& account_id : account_ids) {
      accounts.push_back(load_directory_entry(account_id));
    }
    return accounts;
  }

  template<typename C>
  std::vector<DirectoryEntry>
      SqlServiceLocatorDataStore<C>::load_all_directories() {
    auto directory_ids = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("id"),
        "directories", std::back_inserter(directory_ids)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    auto directories = std::vector<DirectoryEntry>();
    for(auto& directory_id : directory_ids) {
      directories.push_back(load_directory_entry(directory_id));
    }
    return directories;
  }

  template<typename C>
  DirectoryEntry
      SqlServiceLocatorDataStore<C>::load_account(const std::string& name) {
    auto id = std::optional<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("id"),
        "accounts", Viper::sym("name") == name, &id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    if(!id) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    return DirectoryEntry::make_account(*id, name);
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      boost::posix_time::ptime registration_time) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Parent must be a directory."));
    }
    auto account_exists = [&] {
      try {
        load_account(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(account_exists) {
      boost::throw_with_location(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto entry = DirectoryEntry::make_account(load_next_entry_id(), name);
    auto row = AccountsRow(entry, hash_password(entry, password),
      registration_time, boost::posix_time::neg_infin);
    try {
      m_connection->execute(
        Viper::insert(get_accounts_row(), "accounts", &row));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    associate(entry, parent);
    return entry;
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Parent must be a directory."));
    }
    auto account_exists = [&] {
      try {
        load_account(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(account_exists) {
      boost::throw_with_location(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto entry = DirectoryEntry::make_directory(load_next_entry_id(), name);
    try {
      m_connection->execute(
        Viper::insert(get_directories_row(), "directories", &entry));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    if(parent.m_id != -1 || entry.m_id != 0) {
      associate(entry, parent);
    }
    return entry;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::remove(const DirectoryEntry& entry) {
    auto children = load_children(entry);
    if(!children.empty()) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Directory entry is not empty."));
    }
    try {
      m_connection->execute(Viper::erase("permissions",
        Viper::sym("source") == entry.m_id ||
        Viper::sym("target") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    auto parents = load_parents(entry);
    for(auto& parent : parents) {
      detach(entry, parent);
    }
    auto table = [&] {
      if(entry.m_type == DirectoryEntry::Type::ACCOUNT) {
        return "accounts";
      } else {
        return "directories";
      }
    }();
    try {
      m_connection->execute(
        Viper::erase(table, Viper::sym("id") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  bool SqlServiceLocatorDataStore<C>::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto validated_parent = load_directory_entry(parent.m_id);
    if(validated_parent.m_id == -1) {
      return false;
    }
    if(validated_parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Parent not found."));
    }
    if(entry == validated_parent) {
      return false;
    }
    auto parents = load_parents(entry);
    if(std::ranges::contains(parents, validated_parent)) {
      return false;
    }
    try {
      auto parents_row = ParentsRow(entry.m_id, validated_parent.m_id);
      m_connection->execute(
        Viper::insert(get_parents_row(), "parents", &parents_row));
      auto children_row = ChildrenRow(validated_parent.m_id, entry.m_id);
      m_connection->execute(
        Viper::insert(get_children_row(), "children", &children_row));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    return true;
  }

  template<typename C>
  bool SqlServiceLocatorDataStore<C>::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Parent not found."));
    }
    if(entry == parent) {
      return false;
    }
    auto parents = load_parents(entry);
    if(!std::ranges::contains(parents, parent)) {
      return false;
    }
    try {
      m_connection->execute(
        Viper::erase("parents", Viper::sym("entry") == entry.m_id &&
          Viper::sym("parent") == parent.m_id));
      m_connection->execute(
        Viper::erase("children", Viper::sym("entry") == parent.m_id &&
          Viper::sym("child") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    return true;
  }

  template<typename C>
  std::string SqlServiceLocatorDataStore<C>::load_password(
      const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    auto password = std::string();
    try {
      m_connection->execute(Viper::select(Viper::Row<std::string>("password"),
        "accounts", Viper::sym("id") == account.m_id, &password));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    return password;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::set_password(
      const DirectoryEntry& account, const std::string& password) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    try {
      m_connection->execute(Viper::update("accounts", {"password", password},
        Viper::sym("id") == account.m_id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  Permissions SqlServiceLocatorDataStore<C>::load_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    try {
      auto permission = std::optional<unsigned int>();
      m_connection->execute(Viper::select(
        Viper::Row<unsigned int>("permission"), "permissions",
        Viper::sym("source") == source.m_id &&
          Viper::sym("target") == target.m_id, &permission));
      if(!permission) {
        return Permission::NONE;
      }
      return Permissions::from_bitmask(*permission);
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      SqlServiceLocatorDataStore<C>::load_all_permissions(
      const DirectoryEntry& account) {
    auto rows = std::vector<PermissionsRow>();
    try {
      m_connection->execute(Viper::select(get_permissions_row(), "permissions",
        Viper::sym("source") == account.m_id, std::back_inserter(rows)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
    auto permissions = std::vector<std::tuple<DirectoryEntry, Permissions>>();
    for(auto& row : rows) {
      permissions.emplace_back(load_directory_entry(row.m_target),
        Permissions::from_bitmask(row.m_permission));
    }
    return permissions;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::set_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto row = PermissionsRow(source.m_id, target.m_id,
      static_cast<unsigned int>(permissions.get_bitset().to_ulong()));
    try {
      m_connection->execute(
        Viper::upsert(get_permissions_row(), "permissions", &row));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  boost::posix_time::ptime SqlServiceLocatorDataStore<C>::load_registration_time(
      const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    try {
      auto registration_time = boost::posix_time::ptime();
      m_connection->execute(Viper::select(Viper::Row<boost::posix_time::ptime>(
        "registration_time"), "accounts", Viper::sym("id") == account.m_id,
        &registration_time));
      return registration_time;
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  boost::posix_time::ptime SqlServiceLocatorDataStore<C>::load_last_login_time(
      const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    try {
      auto last_login_time = boost::posix_time::ptime();
      m_connection->execute(Viper::select(Viper::Row<boost::posix_time::ptime>(
        "last_login_time"), "accounts", Viper::sym("id") == account.m_id,
        &last_login_time));
      return last_login_time;
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::store_last_login_time(
      const DirectoryEntry& account, boost::posix_time::ptime login_time) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    try {
      m_connection->execute(Viper::update("accounts",
        {"last_login_time", login_time}, Viper::sym("id") == account.m_id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::rename(
      const DirectoryEntry& entry, const std::string& name) {
    try {
      m_connection->execute(Viper::update(
        "accounts", {"name", name}, Viper::sym("id") == entry.m_id));
      m_connection->execute(Viper::update(
        "directories", {"name", name}, Viper::sym("id") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<std::invocable<> F>
  decltype(auto) SqlServiceLocatorDataStore<C>::with_transaction(
      F&& transaction) {
    auto lock = std::lock_guard(m_mutex);
    return Viper::transaction(*m_connection, std::forward<F>(transaction));
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_connection->close();
    m_open_state.close();
  }

  template<typename C>
  unsigned int SqlServiceLocatorDataStore<C>::load_next_entry_id() {
    auto result = m_next_entry_id;
    m_connection->execute(
      Viper::update("settings", {"next_entry_id", m_next_entry_id + 1}));
    ++m_next_entry_id;
    return result;
  }
}

#endif
