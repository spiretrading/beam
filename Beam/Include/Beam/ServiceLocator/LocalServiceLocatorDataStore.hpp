#ifndef BEAM_LOCAL_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_LOCAL_SERVICE_LOCATOR_DATA_STORE_HPP
#include <tuple>
#include <unordered_map>
#include <boost/range/adaptor/map.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Utilities/HashTuple.hpp"

namespace Beam::ServiceLocator {

  /** Implements the ServiceLocatorDataStore using local memory. */
  class LocalServiceLocatorDataStore : public ServiceLocatorDataStore {
    public:

      /** Constructs a LocalServiceLocatorDataStore. */
      LocalServiceLocatorDataStore();

      ~LocalServiceLocatorDataStore() override;

      /**
       * Stores an account.
       * @param account The account to store.
       * @param password The account's password.
       * @param registration The time the account was registered.
       * @param lastLoginTime The last time the account logged in.
       */
      void Store(DirectoryEntry account, std::string password,
        boost::posix_time::ptime registrationTime,
        boost::posix_time::ptime lastLoginTime);

      /**
       * Stores a directory.
       * @param directory The directory to store.
       */
      void Store(DirectoryEntry directory);

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

      void WithTransaction(
        const std::function<void ()>& transaction) override;

      void Open() override;

      void Close() override;

    private:
      struct AccountEntry {
        DirectoryEntry m_entry;
        boost::posix_time::ptime m_registrationTime;
        boost::posix_time::ptime m_lastLoginTime;
        std::string m_password;
        std::unordered_map<DirectoryEntry, Permissions> m_permissions;
      };
      using DirectoryEntryPair = std::tuple<DirectoryEntry, DirectoryEntry>;
      mutable Threading::Mutex m_mutex;
      unsigned int m_nextId;
      std::unordered_map<unsigned int, std::shared_ptr<AccountEntry>>
        m_idToAccounts;
      std::unordered_map<std::string, std::shared_ptr<AccountEntry>>
        m_nameToAccounts;
      std::unordered_map<unsigned int, DirectoryEntry> m_idToDirectories;
      std::unordered_map<DirectoryEntry, std::vector<DirectoryEntry>> m_parents;
      std::unordered_map<DirectoryEntry, std::vector<DirectoryEntry>>
        m_children;
      IO::OpenState m_openState;

      std::shared_ptr<AccountEntry> FindAccountEntry(
        const DirectoryEntry& entry);
  };

  inline LocalServiceLocatorDataStore::LocalServiceLocatorDataStore()
    : m_nextId(0) {}

  inline LocalServiceLocatorDataStore::~LocalServiceLocatorDataStore() {
    Close();
  }

  inline void LocalServiceLocatorDataStore::Store(DirectoryEntry account,
      std::string password, boost::posix_time::ptime registrationTime,
      boost::posix_time::ptime lastLoginTime) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(
        ServiceLocatorDataStoreException("Invalid account."));
    }
    auto entry = std::make_shared<AccountEntry>();
    entry->m_entry = std::move(account);
    entry->m_registrationTime = registrationTime;
    entry->m_lastLoginTime = lastLoginTime;
    entry->m_password = std::move(password);
    m_nextId = std::max(m_nextId, entry->m_entry.m_id + 1);
    m_idToAccounts.insert(std::make_pair(entry->m_entry.m_id, entry));
    m_nameToAccounts.insert(std::make_pair(entry->m_entry.m_name, entry));
  }

  inline void LocalServiceLocatorDataStore::Store(DirectoryEntry directory) {
    if(directory.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(
        ServiceLocatorDataStoreException("Invalid directory."));
    }
    m_nextId = std::max(m_nextId, directory.m_id + 1);
    m_idToDirectories.insert(std::make_pair(directory.m_id, directory));
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::LoadParents(
      const DirectoryEntry& entry) {
    auto parentsIterator = m_parents.find(entry);
    if(parentsIterator == m_parents.end()) {
      return {};
    }
    return parentsIterator->second;
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::LoadChildren(
      const DirectoryEntry& directory) {
    auto childrenIterator = m_children.find(directory);
    if(childrenIterator == m_children.end()) {
      return {};
    }
    return childrenIterator->second;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::LoadDirectoryEntry(
      unsigned int id) {
    auto accountIterator = m_idToAccounts.find(id);
    if(accountIterator != m_idToAccounts.end()) {
      return accountIterator->second->m_entry;
    }
    auto directoryIterator = m_idToDirectories.find(id);
    if(directoryIterator != m_idToDirectories.end()) {
      return directoryIterator->second;
    }
    BOOST_THROW_EXCEPTION(
      ServiceLocatorDataStoreException("Directory entry not found."));
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::
      LoadAllAccounts() {
    auto accounts = std::vector<DirectoryEntry>();
    accounts.reserve(m_idToAccounts.size());
    std::transform(m_idToAccounts.begin(), m_idToAccounts.end(),
      std::back_inserter(accounts),
      [] (auto& account) {
        return account.second->m_entry;
      });
    return accounts;
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::
      LoadAllDirectories() {
    auto directories = std::vector<DirectoryEntry>();
    directories.reserve(m_idToDirectories.size());
    std::transform(m_idToDirectories.begin(), m_idToDirectories.end(),
      std::back_inserter(directories),
      [] (auto& directory) {
        return directory.second;
      });
    return directories;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::LoadAccount(
      const std::string& name) {
    auto accountIterator = m_nameToAccounts.find(name);
    if(accountIterator == m_nameToAccounts.end()) {
      BOOST_THROW_EXCEPTION(
        ServiceLocatorDataStoreException("Account not found."));
    }
    return accountIterator->second->m_entry;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent, boost::posix_time::ptime registrationTime) {
    auto accountExists =
      [&] {
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
    auto newEntry = DirectoryEntry::MakeAccount(m_nextId, name);
    Store(newEntry, HashPassword(newEntry, password), registrationTime,
      boost::posix_time::neg_infin);
    Associate(newEntry, parent);
    return newEntry;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    auto accountExists =
      [&] {
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
    auto newEntry = DirectoryEntry::MakeDirectory(m_nextId, name);
    Store(newEntry);
    Associate(newEntry, parent);
    return newEntry;
  }

  inline void LocalServiceLocatorDataStore::Delete(
      const DirectoryEntry& entry) {
    auto children = LoadChildren(entry);
    if(!children.empty()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Directory entry is not empty."));
    }
    for(auto& accountEntry : m_idToAccounts | boost::adaptors::map_values) {
      accountEntry->m_permissions.erase(entry);
    }
    auto parentsIterator = m_parents.find(entry);
    if(parentsIterator != m_parents.end()) {
      auto parents = parentsIterator->second;
      for(auto& parent : parents) {
        Detach(entry, parent);
      }
      m_parents.erase(parentsIterator);
    }
    auto accountEntry = m_idToAccounts.find(entry.m_id);
    if(accountEntry != m_idToAccounts.end()) {
      m_nameToAccounts.erase(accountEntry->second->m_entry.m_name);
    }
    m_idToAccounts.erase(entry.m_id);
    m_idToDirectories.erase(entry.m_id);
  }

  inline bool LocalServiceLocatorDataStore::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    if(parent.m_id == -1) {
      return false;
    }
    auto parentIterator = m_idToDirectories.find(parent.m_id);
    if(parentIterator == m_idToDirectories.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent not found."));
    }
    if(entry != LoadDirectoryEntry(entry.m_id)) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Entry not found."));
    }
    if(parent == entry) {
      return false;
    }
    auto& parents = m_parents[entry];
    auto parentsIterator = std::find(parents.begin(), parents.end(), parent);
    if(parentsIterator == parents.end()) {
      parents.push_back(parent);
    }
    auto& children = m_children[parent];
    auto childrenIterator = std::find(children.begin(), children.end(), entry);
    if(childrenIterator == children.end()) {
      children.push_back(entry);
      return true;
    }
    return false;
  }

  inline bool LocalServiceLocatorDataStore::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(m_idToDirectories.find(parent.m_id) == m_idToDirectories.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent not found."));
    }
    if(entry != LoadDirectoryEntry(entry.m_id)) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Entry not found."));
    }
    auto parentsIterator = m_parents.find(entry);
    if(parentsIterator == m_parents.end()) {
      return false;
    }
    auto& parents = parentsIterator->second;
    auto parentIterator = std::find(parents.begin(), parents.end(), parent);
    if(parentIterator == parents.end()) {
      return false;
    }
    parents.erase(parentIterator);
    auto& children = m_children[parent];
    children.erase(std::find(children.begin(), children.end(), entry));
    return true;
  }

  inline std::string LocalServiceLocatorDataStore::LoadPassword(
      const DirectoryEntry& account) {
    auto accountEntry = FindAccountEntry(account);
    return accountEntry->m_password;
  }

  inline void LocalServiceLocatorDataStore::SetPassword(
      const DirectoryEntry& account, const std::string& password) {
    auto accountEntry = FindAccountEntry(account);
    accountEntry->m_password = password;
  }

  inline Permissions LocalServiceLocatorDataStore::LoadPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    auto accountEntry = m_idToAccounts.find(source.m_id);
    if(accountEntry == m_idToAccounts.end()) {
      return Permission::NONE;
    }
    auto permissionsIterator = accountEntry->second->m_permissions.find(target);
    if(permissionsIterator == accountEntry->second->m_permissions.end()) {
      return Permission::NONE;
    }
    return permissionsIterator->second;
  }

  inline std::vector<std::tuple<DirectoryEntry, Permissions>>
      LocalServiceLocatorDataStore::LoadAllPermissions(
      const DirectoryEntry& account) {
    auto accountEntry = m_idToAccounts.find(account.m_id);
    if(accountEntry == m_idToAccounts.end()) {
      return {};
    }
    auto permissions = std::vector<std::tuple<DirectoryEntry, Permissions>>();
    for(auto& pair : accountEntry->second->m_permissions) {
      permissions.emplace_back(pair.first, pair.second);
    }
    return permissions;
  }

  inline void LocalServiceLocatorDataStore::SetPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto accountEntry = m_idToAccounts.find(source.m_id);
    if(accountEntry == m_idToAccounts.end()) {
      return;
    }
    if(permissions == Permissions{}) {
      accountEntry->second->m_permissions.erase(target);
    } else {
      accountEntry->second->m_permissions[target] = permissions;
    }
  }

  inline boost::posix_time::ptime LocalServiceLocatorDataStore::
      LoadRegistrationTime(const DirectoryEntry& account) {
    auto accountEntry = FindAccountEntry(account);
    return accountEntry->m_registrationTime;
  }

  inline boost::posix_time::ptime LocalServiceLocatorDataStore::
      LoadLastLoginTime(const DirectoryEntry& account) {
    auto accountEntry = FindAccountEntry(account);
    return accountEntry->m_lastLoginTime;
  }

  inline void LocalServiceLocatorDataStore::StoreLastLoginTime(
      const DirectoryEntry& account, boost::posix_time::ptime loginTime) {
    auto accountEntry = FindAccountEntry(account);
    accountEntry->m_lastLoginTime = loginTime;
  }

  inline void LocalServiceLocatorDataStore::Rename(const DirectoryEntry& entry,
      const std::string& name) {
    auto accountIdIterator = m_idToAccounts.find(entry.m_id);
    if(accountIdIterator != m_idToAccounts.end()) {
      m_nameToAccounts.erase(accountIdIterator->second->m_entry.m_name);
      accountIdIterator->second->m_entry.m_name = name;
      m_nameToAccounts[name] = accountIdIterator->second;
    }
    auto directoryIdIterator = m_idToDirectories.find(entry.m_id);
    if(directoryIdIterator != m_idToDirectories.end()) {
      directoryIdIterator->second.m_name = name;
    }
    for(auto& parents : m_parents) {
      for(auto& parent : parents.second) {
        if(parent.m_id == entry.m_id) {
          parent.m_name = name;
        }
      }
    }
    for(auto& children : m_children) {
      for(auto& child : children.second) {
        if(child.m_id == entry.m_id) {
          child.m_name = name;
        }
      }
    }
  }

  inline void LocalServiceLocatorDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    auto lock = boost::lock_guard(m_mutex);
    transaction();
  }

  inline void LocalServiceLocatorDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalServiceLocatorDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }

  inline std::shared_ptr<LocalServiceLocatorDataStore::AccountEntry>
      LocalServiceLocatorDataStore::FindAccountEntry(
      const DirectoryEntry& entry) {
    auto accountIterator = m_idToAccounts.find(entry.m_id);
    if(accountIterator == m_idToAccounts.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    return accountIterator->second;
  }
}

#endif
