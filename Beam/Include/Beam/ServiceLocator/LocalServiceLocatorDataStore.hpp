#ifndef BEAM_LOCALSERVICELOCATORDATASTORE_HPP
#define BEAM_LOCALSERVICELOCATORDATASTORE_HPP
#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/Utilities/HashTuple.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class LocalServiceLocatorDataStore
      \brief Implements the ServiceLocatorDataStore using local memory.
   */
  class LocalServiceLocatorDataStore : public ServiceLocatorDataStore {
    public:

      //! Constructs a LocalServiceLocatorDataStore.
      LocalServiceLocatorDataStore();

      virtual ~LocalServiceLocatorDataStore();

      //! Stores an account.
      /*!
        \param account The account to store.
        \param password The account's password.
        \param registration The time the account was registered.
        \param lastLoginTime The last time the account logged in.
      */
      void Store(DirectoryEntry account, std::string password,
        boost::posix_time::ptime registrationTime,
        boost::posix_time::ptime lastLoginTime);

      //! Stores a directory.
      /*!
        \param directory The directory to store.
      */
      void Store(DirectoryEntry directory);

      virtual std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry);

      virtual std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& directory);

      virtual DirectoryEntry LoadDirectoryEntry(unsigned int id);

      virtual std::vector<DirectoryEntry> LoadAllAccounts();

      virtual std::vector<DirectoryEntry> LoadAllDirectories();

      virtual DirectoryEntry LoadAccount(const std::string& name);

      virtual DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent,
        const boost::posix_time::ptime& registrationTime);

      virtual DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent);

      virtual void Delete(const DirectoryEntry& entry);

      virtual bool Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent);

      virtual bool Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent);

      virtual std::string LoadPassword(const DirectoryEntry& account);

      virtual void SetPassword(const DirectoryEntry& account,
        const std::string& password);

      virtual Permissions LoadPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target);

      virtual void SetPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions);

      virtual boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account);

      virtual boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account);

      virtual void StoreLastLoginTime(const DirectoryEntry& account,
        const boost::posix_time::ptime& loginTime);

      virtual void WithTransaction(const std::function<void ()>& transaction);

      virtual void Open();

      virtual void Close();

    private:
      typedef boost::tuple<DirectoryEntry, DirectoryEntry> DirectoryEntryPair;
      mutable boost::mutex m_mutex;
      unsigned int m_nextId;
      std::vector<DirectoryEntry> m_accounts;
      std::vector<DirectoryEntry> m_directories;
      std::unordered_map<DirectoryEntry, boost::posix_time::ptime>
        m_registrationTimes;
      std::unordered_map<DirectoryEntry, boost::posix_time::ptime>
        m_lastLoginTimes;
      std::unordered_map<DirectoryEntry, std::vector<DirectoryEntry>> m_parents;
      std::unordered_map<DirectoryEntry, std::vector<DirectoryEntry>>
        m_children;
      std::unordered_map<DirectoryEntryPair, Permissions> m_permissions;
      std::unordered_map<DirectoryEntry, std::string> m_passwords;
      IO::OpenState m_openState;
  };

  inline LocalServiceLocatorDataStore::LocalServiceLocatorDataStore()
      : m_nextId(0) {}

  inline LocalServiceLocatorDataStore::~LocalServiceLocatorDataStore() {
    Close();
  }

  inline void LocalServiceLocatorDataStore::Store(DirectoryEntry account,
      std::string password, boost::posix_time::ptime registrationTime,
      boost::posix_time::ptime lastLoginTime) {
    m_accounts.push_back(std::move(account));
    m_passwords[m_accounts.back()] = std::move(password);
    m_registrationTimes[m_accounts.back()] = registrationTime;
    m_lastLoginTimes[m_accounts.back()] = lastLoginTime;
  }

  inline void LocalServiceLocatorDataStore::Store(DirectoryEntry directory) {
    m_directories.push_back(std::move(directory));
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::LoadParents(
      const DirectoryEntry& entry) {
    auto parentsIterator = m_parents.find(entry);
    if(parentsIterator == m_parents.end()) {
      return std::vector<DirectoryEntry>();
    }
    return parentsIterator->second;
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::LoadChildren(
      const DirectoryEntry& directory) {
    auto childrenIterator = m_children.find(directory);
    if(childrenIterator == m_children.end()) {
      return std::vector<DirectoryEntry>();
    }
    return childrenIterator->second;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::LoadDirectoryEntry(
      unsigned int id) {
    for(const DirectoryEntry& account : m_accounts) {
      if(account.m_id == id) {
        return account;
      }
    }
    for(const DirectoryEntry& directory : m_directories) {
      if(directory.m_id == id) {
        return directory;
      }
    }
    BOOST_THROW_EXCEPTION(
      ServiceLocatorDataStoreException("Directory entry not found."));
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::
      LoadAllAccounts() {
    return m_accounts;
  }

  inline std::vector<DirectoryEntry> LocalServiceLocatorDataStore::
      LoadAllDirectories() {
    return m_directories;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::LoadAccount(
      const std::string& name) {
    for(const DirectoryEntry& account : m_accounts) {
      if(account.m_name == name) {
        return account;
      }
    }
    BOOST_THROW_EXCEPTION(
      ServiceLocatorDataStoreException("Account not found."));
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      const boost::posix_time::ptime& registrationTime) {
    bool accountExists;
    try {
      DirectoryEntry existingAccount = LoadAccount(name);
      accountExists = true;
    } catch(const ServiceLocatorDataStoreException&) {
      accountExists = false;
    }
    if(accountExists) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    DirectoryEntry newEntry(DirectoryEntry::Type::ACCOUNT, m_nextId, name);
    ++m_nextId;
    m_accounts.push_back(newEntry);
    m_passwords.insert(std::make_pair(newEntry,
      HashPassword(newEntry, password)));
    m_registrationTimes.insert(std::make_pair(newEntry, registrationTime));
    m_lastLoginTimes.insert(std::make_pair(newEntry,
      boost::posix_time::neg_infin));
    Associate(newEntry, parent);
    return newEntry;
  }

  inline DirectoryEntry LocalServiceLocatorDataStore::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    bool accountExists;
    try {
      DirectoryEntry existingAccount = LoadAccount(name);
      accountExists = true;
    } catch(const ServiceLocatorDataStoreException&) {
      accountExists = false;
    }
    if(accountExists) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    DirectoryEntry newEntry(DirectoryEntry::Type::DIRECTORY, m_nextId, name);
    ++m_nextId;
    m_directories.push_back(newEntry);
    Associate(newEntry, parent);
    return newEntry;
  }

  inline void LocalServiceLocatorDataStore::Delete(
      const DirectoryEntry& entry) {
    std::vector<DirectoryEntry> children = LoadChildren(entry);
    if(!children.empty()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Directory entry is not empty."));
    }
    auto permissionsIterator = m_permissions.begin();
    while(permissionsIterator != m_permissions.end()) {
      const auto& permissionsPair = permissionsIterator->first;
      if(boost::get<0>(permissionsPair) == entry ||
          boost::get<1>(permissionsPair) == entry) {
        permissionsIterator = m_permissions.erase(permissionsIterator);
      } else {
        ++permissionsIterator;
      }
    }
    auto parentsIterator = m_parents.find(entry);
    if(parentsIterator != m_parents.end()) {
      std::vector<DirectoryEntry> parents = parentsIterator->second;
      for(const DirectoryEntry& parent : parents) {
        Detach(entry, parent);
      }
      m_parents.erase(parentsIterator);
    }
    if(entry.m_type == DirectoryEntry::Type::ACCOUNT) {
      for(auto i = m_accounts.begin(); i != m_accounts.end(); ++i) {
        if(*i == entry) {
          m_accounts.erase(i);
          m_passwords.erase(entry);
          break;
        }
      }
    } else {
      for(auto i = m_directories.begin(); i != m_directories.end(); ++i) {
        if(*i == entry) {
          m_directories.erase(i);
          break;
        }
      }
    }
  }

  inline bool LocalServiceLocatorDataStore::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    if(parent.m_id == -1) {
      return false;
    }
    auto parentIterator = std::find(m_directories.begin(), m_directories.end(),
      parent);
    if(parentIterator == m_directories.end()) {
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
    std::vector<DirectoryEntry>& parents = m_parents[entry];
    auto parentsIterator = std::find(parents.begin(), parents.end(), parent);
    if(parentsIterator == parents.end()) {
      parents.push_back(parent);
    }
    std::vector<DirectoryEntry>& children = m_children[parent];
    auto childrenIterator = std::find(children.begin(), children.end(), entry);
    if(childrenIterator == children.end()) {
      children.push_back(entry);
      return true;
    }
    return false;
  }

  inline bool LocalServiceLocatorDataStore::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(std::find(m_directories.begin(), m_directories.end(), parent) ==
        m_directories.end()) {
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
    std::vector<DirectoryEntry>& parents = parentsIterator->second;
    auto parentIterator = std::find(parents.begin(), parents.end(), parent);
    if(parentIterator == parents.end()) {
      return false;
    }
    parents.erase(parentIterator);
    std::vector<DirectoryEntry>& children = m_children[parent];
    children.erase(std::find(children.begin(), children.end(), entry));
    return true;
  }

  inline std::string LocalServiceLocatorDataStore::LoadPassword(
      const DirectoryEntry& account) {
    auto passwordIterator = m_passwords.find(account);
    if(passwordIterator == m_passwords.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    return passwordIterator->second;
  }

  inline void LocalServiceLocatorDataStore::SetPassword(
      const DirectoryEntry& account, const std::string& password) {
    auto passwordIterator = m_passwords.find(account);
    if(passwordIterator == m_passwords.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    passwordIterator->second = password;
  }

  inline Permissions LocalServiceLocatorDataStore::LoadPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    DirectoryEntryPair key(source, target);
    auto permissionsIterator = m_permissions.find(key);
    if(permissionsIterator == m_permissions.end()) {
      return Permission::NONE;
    }
    return permissionsIterator->second;
  }

  inline void LocalServiceLocatorDataStore::SetPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    DirectoryEntryPair pair(source, target);
    m_permissions[pair] = permissions;
  }

  inline boost::posix_time::ptime LocalServiceLocatorDataStore::
      LoadRegistrationTime(const DirectoryEntry& account) {
    auto accountIterator = std::find(m_accounts.begin(), m_accounts.end(),
      account);
    if(accountIterator == m_accounts.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    return m_registrationTimes[*accountIterator];
  }

  inline boost::posix_time::ptime LocalServiceLocatorDataStore::
      LoadLastLoginTime(const DirectoryEntry& account) {
    auto accountIterator = std::find(m_accounts.begin(), m_accounts.end(),
      account);
    if(accountIterator == m_accounts.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    auto lastLoginTimesIterator = m_lastLoginTimes.find(*accountIterator);
    if(lastLoginTimesIterator == m_lastLoginTimes.end()) {
      return boost::posix_time::neg_infin;
    }
    return lastLoginTimesIterator->second;
  }

  inline void LocalServiceLocatorDataStore::StoreLastLoginTime(
      const DirectoryEntry& account,
      const boost::posix_time::ptime& loginTime) {
    auto accountIterator = std::find(m_accounts.begin(), m_accounts.end(),
      account);
    if(accountIterator == m_accounts.end()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    m_lastLoginTimes[*accountIterator] = loginTime;
  }

  inline void LocalServiceLocatorDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
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
}
}

#endif
