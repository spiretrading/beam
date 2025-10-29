#ifndef BEAM_LOCAL_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_LOCAL_SERVICE_LOCATOR_DATA_STORE_HPP
#include <tuple>
#include <unordered_map>
#include <boost/range/adaptor/map.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Utilities/HashTuple.hpp"

namespace Beam {

  /** Implements the ServiceLocatorDataStore using local memory. */
  class LocalServiceLocatorDataStore {
    public:

      /** Constructs a LocalServiceLocatorDataStore. */
      LocalServiceLocatorDataStore();

      ~LocalServiceLocatorDataStore();

      /**
       * Stores an account.
       * @param account The account to store.
       * @param password The account's password.
       * @param registration_time The time the account was registered.
       * @param last_login_time The last time the account logged in.
       */
      void store(DirectoryEntry account, std::string password,
        boost::posix_time::ptime registration_time,
        boost::posix_time::ptime last_login_time);

      /**
       * Stores a directory.
       * @param directory The directory to store.
       */
      void store(DirectoryEntry directory);

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
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);
      void close();

    private:
      struct AccountEntry {
        DirectoryEntry m_entry;
        boost::posix_time::ptime m_registration_time;
        boost::posix_time::ptime m_last_login_time;
        std::string m_password;
        std::unordered_map<DirectoryEntry, Permissions> m_permissions;
      };
      using DirectoryEntryPair = std::tuple<DirectoryEntry, DirectoryEntry>;
      mutable Mutex m_mutex;
      unsigned int m_next_id;
      std::unordered_map<unsigned int, std::shared_ptr<AccountEntry>>
        m_id_to_accounts;
      std::unordered_map<std::string, std::shared_ptr<AccountEntry>>
        m_name_to_accounts;
      std::unordered_map<unsigned int, DirectoryEntry> m_id_to_directories;
      std::unordered_map<DirectoryEntry, std::vector<DirectoryEntry>> m_parents;
      std::unordered_map<DirectoryEntry, std::vector<DirectoryEntry>>
        m_children;
      OpenState m_open_state;

      std::shared_ptr<AccountEntry> find_account_entry(
        const DirectoryEntry& entry);
  };

  inline LocalServiceLocatorDataStore::LocalServiceLocatorDataStore()
    : m_next_id(0) {}

  inline LocalServiceLocatorDataStore::~LocalServiceLocatorDataStore() {
    close();
  }

  inline void LocalServiceLocatorDataStore::store(DirectoryEntry account,
      std::string password, boost::posix_time::ptime registration_time,
      boost::posix_time::ptime last_login_time) {
    m_open_state.ensure_open();
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Invalid account."));
    }
    auto entry = std::make_shared<AccountEntry>();
    entry->m_entry = std::move(account);
    entry->m_registration_time = registration_time;
    entry->m_last_login_time = last_login_time;
    entry->m_password = std::move(password);
    m_next_id = std::max(m_next_id, entry->m_entry.m_id + 1);
    m_id_to_accounts.insert(std::pair(entry->m_entry.m_id, entry));
    m_name_to_accounts.insert(std::pair(entry->m_entry.m_name, entry));
  }

  inline void LocalServiceLocatorDataStore::store(DirectoryEntry directory) {
    m_open_state.ensure_open();
    if(directory.m_type != DirectoryEntry::Type::DIRECTORY) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Invalid directory."));
    }
    m_next_id = std::max(m_next_id, directory.m_id + 1);
    m_id_to_directories.insert(std::pair(directory.m_id, directory));
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::load_parents(
      const DirectoryEntry& entry) {
    m_open_state.ensure_open();
    auto parent = m_parents.find(entry);
    if(parent == m_parents.end()) {
      return {};
    }
    return parent->second;
  }

  inline std::vector<DirectoryEntry>
      LocalServiceLocatorDataStore::load_children(
        const DirectoryEntry& directory) {
    m_open_state.ensure_open();
    auto child = m_children.find(directory);
    if(child == m_children.end()) {
      return {};
    }
    return child->second;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::load_directory_entry(
      unsigned int id) {
    m_open_state.ensure_open();
    auto account = m_id_to_accounts.find(id);
    if(account != m_id_to_accounts.end()) {
      return account->second->m_entry;
    }
    auto directory = m_id_to_directories.find(id);
    if(directory != m_id_to_directories.end()) {
      return directory->second;
    }
    boost::throw_with_location(
      ServiceLocatorDataStoreException("Directory entry not found."));
  }

  inline std::vector<DirectoryEntry>
      LocalServiceLocatorDataStore::load_all_accounts() {
    m_open_state.ensure_open();
    auto accounts = std::vector<DirectoryEntry>();
    accounts.reserve(m_id_to_accounts.size());
    std::transform(m_id_to_accounts.begin(), m_id_to_accounts.end(),
      std::back_inserter(accounts), [] (const auto& account) {
        return account.second->m_entry;
      });
    return accounts;
  }

  inline std::vector<DirectoryEntry>
      LocalServiceLocatorDataStore::load_all_directories() {
    m_open_state.ensure_open();
    auto directories = std::vector<DirectoryEntry>();
    directories.reserve(m_id_to_directories.size());
    std::transform(m_id_to_directories.begin(), m_id_to_directories.end(),
      std::back_inserter(directories), [] (const auto& directory) {
        return directory.second;
      });
    return directories;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::load_account(
      const std::string& name) {
    m_open_state.ensure_open();
    auto account = m_name_to_accounts.find(name);
    if(account == m_name_to_accounts.end()) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    return account->second->m_entry;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent, boost::posix_time::ptime registration_time) {
    m_open_state.ensure_open();
    auto exists = [&] {
      try {
        load_account(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(exists) {
      boost::throw_with_location(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto new_entry = DirectoryEntry::make_account(m_next_id, name);
    store(new_entry, hash_password(new_entry, password), registration_time,
      boost::posix_time::neg_infin);
    associate(new_entry, parent);
    return new_entry;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    m_open_state.ensure_open();
    auto exists = [&] {
      try {
        load_account(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(exists) {
      boost::throw_with_location(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto new_entry = DirectoryEntry::make_directory(m_next_id, name);
    store(new_entry);
    associate(new_entry, parent);
    return new_entry;
  }

  inline void LocalServiceLocatorDataStore::remove(
      const DirectoryEntry& entry) {
    m_open_state.ensure_open();
    auto children = load_children(entry);
    if(!children.empty()) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Directory entry is not empty."));
    }
    for(auto& account_entry : m_id_to_accounts | boost::adaptors::map_values) {
      account_entry->m_permissions.erase(entry);
    }
    auto parents = m_parents.find(entry);
    if(parents != m_parents.end()) {
      for(auto& parent : parents->second) {
        detach(entry, parent);
      }
      m_parents.erase(parents);
    }
    auto account_entry = m_id_to_accounts.find(entry.m_id);
    if(account_entry != m_id_to_accounts.end()) {
      m_name_to_accounts.erase(account_entry->second->m_entry.m_name);
    }
    m_id_to_accounts.erase(entry.m_id);
    m_id_to_directories.erase(entry.m_id);
  }

  inline bool LocalServiceLocatorDataStore::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_open_state.ensure_open();
    if(parent.m_id == -1) {
      return false;
    }
    if(!m_id_to_directories.contains(parent.m_id)) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Parent not found."));
    }
    if(entry != load_directory_entry(entry.m_id)) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Entry not found."));
    }
    if(parent == entry) {
      return false;
    }
    auto& parents = m_parents[entry];
    if(std::find(parents.begin(), parents.end(), parent) == parents.end()) {
      parents.push_back(parent);
    }
    auto& children = m_children[parent];
    if(std::find(children.begin(), children.end(), entry) == children.end()) {
      children.push_back(entry);
      return true;
    }
    return false;
  }

  inline bool LocalServiceLocatorDataStore::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_open_state.ensure_open();
    if(m_id_to_directories.find(parent.m_id) == m_id_to_directories.end()) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Parent not found."));
    }
    if(entry != load_directory_entry(entry.m_id)) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Entry not found."));
    }
    auto i = m_parents.find(entry);
    if(i == m_parents.end()) {
      return false;
    }
    auto& parents = i->second;
    auto j = std::find(parents.begin(), parents.end(), parent);
    if(j == parents.end()) {
      return false;
    }
    parents.erase(j);
    auto& children = m_children[parent];
    children.erase(std::find(children.begin(), children.end(), entry));
    return true;
  }

  inline std::string LocalServiceLocatorDataStore::load_password(
      const DirectoryEntry& account) {
    m_open_state.ensure_open();
    auto account_entry = find_account_entry(account);
    return account_entry->m_password;
  }

  inline void LocalServiceLocatorDataStore::set_password(
      const DirectoryEntry& account, const std::string& password) {
    m_open_state.ensure_open();
    auto account_entry = find_account_entry(account);
    account_entry->m_password = password;
  }

  inline Permissions LocalServiceLocatorDataStore::load_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    m_open_state.ensure_open();
    auto account_entry = m_id_to_accounts.find(source.m_id);
    if(account_entry == m_id_to_accounts.end()) {
      return Permission::NONE;
    }
    auto permissions = account_entry->second->m_permissions.find(target);
    if(permissions == account_entry->second->m_permissions.end()) {
      return Permission::NONE;
    }
    return permissions->second;
  }

  inline std::vector<std::tuple<DirectoryEntry, Permissions>>
      LocalServiceLocatorDataStore::load_all_permissions(
      const DirectoryEntry& account) {
    m_open_state.ensure_open();
    auto account_entry = m_id_to_accounts.find(account.m_id);
    if(account_entry == m_id_to_accounts.end()) {
      return std::vector<std::tuple<DirectoryEntry, Permissions>>();
    }
    auto permissions = std::vector<std::tuple<DirectoryEntry, Permissions>>();
    for(auto& permission : account_entry->second->m_permissions) {
      permissions.emplace_back(permission.first, permission.second);
    }
    return permissions;
  }

  inline void LocalServiceLocatorDataStore::set_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_open_state.ensure_open();
    auto account_entry = m_id_to_accounts.find(source.m_id);
    if(account_entry == m_id_to_accounts.end()) {
      return;
    }
    if(permissions == Permissions()) {
      account_entry->second->m_permissions.erase(target);
    } else {
      account_entry->second->m_permissions[target] = permissions;
    }
  }

  inline boost::posix_time::ptime LocalServiceLocatorDataStore::
      load_registration_time(const DirectoryEntry& account) {
    m_open_state.ensure_open();
    auto account_entry = find_account_entry(account);
    return account_entry->m_registration_time;
  }

  inline boost::posix_time::ptime LocalServiceLocatorDataStore::
      load_last_login_time(const DirectoryEntry& account) {
    m_open_state.ensure_open();
    auto account_entry = find_account_entry(account);
    return account_entry->m_last_login_time;
  }

  inline void LocalServiceLocatorDataStore::store_last_login_time(
      const DirectoryEntry& account, boost::posix_time::ptime login_time) {
    m_open_state.ensure_open();
    auto account_entry = find_account_entry(account);
    account_entry->m_last_login_time = login_time;
  }

  inline void LocalServiceLocatorDataStore::rename(const DirectoryEntry& entry,
      const std::string& name) {
    m_open_state.ensure_open();
    auto account_id = m_id_to_accounts.find(entry.m_id);
    if(account_id != m_id_to_accounts.end()) {
      m_name_to_accounts.erase(account_id->second->m_entry.m_name);
      account_id->second->m_entry.m_name = name;
      m_name_to_accounts[name] = account_id->second;
    }
    auto directory_id = m_id_to_directories.find(entry.m_id);
    if(directory_id != m_id_to_directories.end()) {
      directory_id->second.m_name = name;
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

  template<std::invocable<> F>
  decltype(auto) LocalServiceLocatorDataStore::with_transaction(
      F&& transaction) {
    auto lock = boost::lock_guard(m_mutex);
    return std::forward<F>(transaction)();
  }

  inline void LocalServiceLocatorDataStore::close() {
    m_open_state.close();
  }

  inline std::shared_ptr<LocalServiceLocatorDataStore::AccountEntry>
      LocalServiceLocatorDataStore::find_account_entry(
        const DirectoryEntry& entry) {
    auto i = m_id_to_accounts.find(entry.m_id);
    if(i == m_id_to_accounts.end()) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Account not found."));
    }
    return i->second;
  }
}

#endif
