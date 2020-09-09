#ifndef BEAM_SQL_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_SQL_SERVICE_LOCATOR_DATA_STORE_HPP
#include <boost/throw_exception.hpp>
#include <Viper/Viper.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/ServiceLocator/SqlDefinitions.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::ServiceLocator {

  /**
   * Implements the ServiceLocatorDataStore using SQL.
   * @param <C> The type of SQL connection to use.
   */
  template<typename C>
  class SqlServiceLocatorDataStore : public ServiceLocatorDataStore {
    public:

      /** The type of SQL connection to use. */
      using Connection = C;

      /**
       * Constructs an SqlServiceLocatorDataStore.
       * @param connection The SQL connection to use.
       */
      explicit SqlServiceLocatorDataStore(
        std::unique_ptr<Connection> connection);

      ~SqlServiceLocatorDataStore() override;

      std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override;

      std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& directory) override;

      DirectoryEntry LoadDirectoryEntry(unsigned int id) override;

      std::vector<DirectoryEntry> LoadAllAccounts() override;

      std::vector<DirectoryEntry> LoadAllDirectories() override;

      DirectoryEntry LoadAccount(const std::string& name) override;

      DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent,
        boost::posix_time::ptime registrationTime) override;

      DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) override;

      void Delete(const DirectoryEntry& entry) override;

      bool Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      bool Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      std::string LoadPassword(const DirectoryEntry& account) override;

      void SetPassword(const DirectoryEntry& account,
        const std::string& password) override;

      Permissions LoadPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target) override;

      std::vector<std::tuple<DirectoryEntry, Permissions>>
        LoadAllPermissions(const DirectoryEntry& account) override;

      void SetPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override;

      boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) override;

      boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) override;

      void StoreLastLoginTime(const DirectoryEntry& account,
        boost::posix_time::ptime loginTime) override;

      void Rename(const DirectoryEntry& entry,
        const std::string& name) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      unsigned int m_nextEntryId;
      IO::OpenState m_openState;

      unsigned int LoadNextEntryId();
  };

  template<typename C>
  SqlServiceLocatorDataStore<C>::SqlServiceLocatorDataStore(
      std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {
    try {
      m_connection->open();
      if(!m_connection->has_table("settings")) {
        m_connection->execute(Viper::create(GetSettingsRow(), "settings"));
        auto firstEntryId = 0UL;
        m_connection->execute(Viper::insert(GetSettingsRow(), "settings",
          &firstEntryId));
      }
      m_connection->execute(Viper::create_if_not_exists(GetAccountsRow(),
        "accounts"));
      m_connection->execute(Viper::create_if_not_exists(GetDirectoriesRow(),
        "directories"));
      m_connection->execute(Viper::create_if_not_exists(GetParentsRow(),
        "parents"));
      m_connection->execute(Viper::create_if_not_exists(GetChildrenRow(),
        "children"));
      m_connection->execute(Viper::create_if_not_exists(GetPermissionsRow(),
        "permissions"));
      m_connection->execute(Viper::select(GetSettingsRow(), "settings",
        &m_nextEntryId));
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C>
  SqlServiceLocatorDataStore<C>::~SqlServiceLocatorDataStore() {
    Close();
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::LoadParents(
      const DirectoryEntry& entry) {
    auto parentIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("parent"),
        "parents", Viper::sym("entry") == entry.m_id,
        std::back_inserter(parentIds)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto parents = std::vector<DirectoryEntry>();
    for(auto& parentId : parentIds) {
      parents.push_back(LoadDirectoryEntry(parentId));
    }
    return parents;
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::LoadChildren(
      const DirectoryEntry& entry) {
    auto childIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("child"),
        "children", Viper::sym("entry") == entry.m_id,
        std::back_inserter(childIds)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto children = std::vector<DirectoryEntry>();
    for(auto& childId : childIds) {
      children.push_back(LoadDirectoryEntry(childId));
    }
    return children;
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::LoadDirectoryEntry(
      unsigned int id) {
    auto entry = std::optional<DirectoryEntry>();
    try {
      m_connection->execute(Viper::select(GetDirectoryEntryRow(), "accounts",
        Viper::sym("id") == id, &entry));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    if(entry) {
      entry->m_type = DirectoryEntry::Type::ACCOUNT;
      return std::move(*entry);
    }
    try {
      m_connection->execute(Viper::select(GetDirectoryEntryRow(), "directories",
        Viper::sym("id") == id, &entry));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    if(entry) {
      entry->m_type = DirectoryEntry::Type::DIRECTORY;
      return std::move(*entry);
    }
    BOOST_THROW_EXCEPTION(
      ServiceLocatorDataStoreException("Directory entry not found."));
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::LoadAllAccounts() {
    auto accountIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("id"),
        "accounts", std::back_inserter(accountIds)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto accounts = std::vector<DirectoryEntry>();
    for(auto& accountId : accountIds) {
      accounts.push_back(LoadDirectoryEntry(accountId));
    }
    return accounts;
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::
      LoadAllDirectories() {
    auto directoryIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("id"),
        "directories", std::back_inserter(directoryIds)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto directories = std::vector<DirectoryEntry>();
    for(auto& directoryId : directoryIds) {
      directories.push_back(LoadDirectoryEntry(directoryId));
    }
    return directories;
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::LoadAccount(
      const std::string& name) {
    auto id = std::optional<unsigned int>();
    try {
      m_connection->execute(Viper::select(Viper::Row<unsigned int>("id"),
        "accounts", Viper::sym("name") == name, &id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    if(!id) {
      BOOST_THROW_EXCEPTION(
        ServiceLocatorDataStoreException("Account not found."));
    }
    return DirectoryEntry::MakeAccount(*id, name);
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent, boost::posix_time::ptime registrationTime) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent must be a directory."));
    }
    auto accountExists = [&] {
      try {
        LoadAccount(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(accountExists) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto entry = DirectoryEntry::MakeAccount(LoadNextEntryId(), name);
    auto row = AccountsRow{entry, HashPassword(entry, password),
      registrationTime, boost::posix_time::neg_infin};
    try {
      m_connection->execute(Viper::insert(GetAccountsRow(), "accounts", &row));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    Associate(entry, parent);
    return entry;
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent must be a directory."));
    }
    auto accountExists = [&] {
      try {
        LoadAccount(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(accountExists) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto entry = DirectoryEntry::MakeDirectory(LoadNextEntryId(), name);
    try {
      m_connection->execute(Viper::insert(GetDirectoriesRow(), "directories",
        &entry));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    Associate(entry, parent);
    return entry;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::Delete(const DirectoryEntry& entry) {
    auto children = LoadChildren(entry);
    if(!children.empty()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Directory entry is not empty."));
    }
    try {
      m_connection->execute(Viper::erase("permissions",
        Viper::sym("source") == entry.m_id ||
        Viper::sym("target") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto parents = LoadParents(entry);
    for(auto& parent : parents) {
      Detach(entry, parent);
    }
    auto table = [&] {
      if(entry.m_type == DirectoryEntry::Type::ACCOUNT) {
        return "accounts";
      } else {
        return "directories";
      }
    }();
    try {
      m_connection->execute(Viper::erase(table,
        Viper::sym("id") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  bool SqlServiceLocatorDataStore<C>::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(parent.m_id == -1) {
      return false;
    }
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent not found."));
    }
    if(entry == parent) {
      return false;
    }
    auto parents = LoadParents(entry);
    if(std::find(parents.begin(), parents.end(), parent) != parents.end()) {
      return false;
    }
    try {
      auto parentsRow = ParentsRow{entry.m_id, parent.m_id};
      m_connection->execute(Viper::insert(GetParentsRow(), "parents",
        &parentsRow));
      auto childrensRow = ChildrenRow{parent.m_id, entry.m_id};
      m_connection->execute(Viper::insert(GetChildrenRow(), "children",
        &childrensRow));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    return true;
  }

  template<typename C>
  bool SqlServiceLocatorDataStore<C>::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent not found."));
    }
    if(entry == parent) {
      return false;
    }
    try {
      m_connection->execute(Viper::erase("parents",
        Viper::sym("entry") == entry.m_id &&
        Viper::sym("parent") == parent.m_id));
      m_connection->execute(Viper::erase("children",
        Viper::sym("entry") == parent.m_id &&
        Viper::sym("child") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    return true;
  }

  template<typename C>
  std::string SqlServiceLocatorDataStore<C>::LoadPassword(
      const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    auto password = std::string();
    try {
      m_connection->execute(Viper::select(Viper::Row<std::string>("password"),
        "accounts", Viper::sym("id") == account.m_id, &password));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    return password;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::SetPassword(const DirectoryEntry& account,
      const std::string& password) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    try {
      m_connection->execute(Viper::update("accounts", {"password", password},
        Viper::sym("id") == account.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  Permissions SqlServiceLocatorDataStore<C>::LoadPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    try {
      auto permission = std::optional<unsigned int>();
      m_connection->execute(Viper::select(
        Viper::Row<unsigned int>("permission"), "permissions",
        Viper::sym("source") == source.m_id && Viper::sym("target") ==
        target.m_id, &permission));
      if(!permission) {
        return Permission::NONE;
      }
      return Permissions::FromRepresentation(*permission);
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      SqlServiceLocatorDataStore<C>::LoadAllPermissions(
      const DirectoryEntry& account) {
    auto rows = std::vector<PermissionsRow>();
    try {
      m_connection->execute(Viper::select(GetPermissionsRow(), "permissions",
        Viper::sym("source") == account.m_id, std::back_inserter(rows)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto permissions = std::vector<std::tuple<DirectoryEntry, Permissions>>();
    for(auto& row : rows) {
      permissions.emplace_back(LoadDirectoryEntry(row.m_target),
        Permissions::FromRepresentation(row.m_permission));
    }
    return permissions;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::SetPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto row = PermissionsRow{source.m_id, target.m_id,
      static_cast<unsigned int>(permissions.GetBitset().to_ulong())};
    try {
      m_connection->execute(Viper::upsert(GetPermissionsRow(), "permissions",
        &row));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  boost::posix_time::ptime SqlServiceLocatorDataStore<C>::LoadRegistrationTime(
      const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    try {
      auto registrationTime = boost::posix_time::ptime();
      m_connection->execute(Viper::select(Viper::Row<boost::posix_time::ptime>(
        "registration_time"), "accounts", Viper::sym("id") == account.m_id,
        &registrationTime));
      return registrationTime;
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  boost::posix_time::ptime SqlServiceLocatorDataStore<C>::LoadLastLoginTime(
      const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    try {
      auto lastLoginTime = boost::posix_time::ptime();
      m_connection->execute(Viper::select(Viper::Row<boost::posix_time::ptime>(
        "last_login_time"), "accounts", Viper::sym("id") == account.m_id,
        &lastLoginTime));
      return lastLoginTime;
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::StoreLastLoginTime(
      const DirectoryEntry& account, boost::posix_time::ptime loginTime) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    try {
      m_connection->execute(Viper::update("accounts",
        {"last_login_time", loginTime}, Viper::sym("id") == account.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::Rename(const DirectoryEntry& entry,
      const std::string& name) {
    try {
      m_connection->execute(Viper::update("accounts", {"name", name},
        Viper::sym("id") == entry.m_id));
      m_connection->execute(Viper::update("directories", {"name", name},
        Viper::sym("id") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::WithTransaction(
      const std::function<void ()>& transaction) {
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, transaction);
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_connection->close();
    m_openState.Close();
  }

  template<typename C>
  unsigned int SqlServiceLocatorDataStore<C>::LoadNextEntryId() {
    auto result = m_nextEntryId;
    m_connection->execute(Viper::update("settings",
      {"next_entry_id", m_nextEntryId + 1}));
    ++m_nextEntryId;
    return result;
  }
}

#endif
