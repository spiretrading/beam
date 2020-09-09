#ifndef BEAM_CACHED_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_CACHED_SERVICE_LOCATOR_DATA_STORE_HPP
#include <unordered_set>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"

namespace Beam::ServiceLocator {

  /**
   * Caches all data from a ServiceLocatorDataStore.
   * @param <D> The type of ServiceLocatorDataStore to cache.
   */
  template<typename D>
  class CachedServiceLocatorDataStore : public ServiceLocatorDataStore {
    public:

      /** The type of ServiceLocatorDataStore to cache. */
      using DataStore = GetTryDereferenceType<D>;

      /**
       * Constructs a CachedServiceLocatorDataStore.
       * @param dataStore The ServiceLocatorDataStore to cache.
       */
      template<typename DF>
      explicit CachedServiceLocatorDataStore(DF&& dataStore);

      ~CachedServiceLocatorDataStore() override;

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
        LoadAllPermissions(const DirectoryEntry& account);

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

      DirectoryEntry Validate(const DirectoryEntry& entry) override;

      void WithTransaction(
        const std::function<void ()>& transaction) override;

      void Close() override;

    private:
      GetOptionalLocalPtr<D> m_dataStore;
      std::unordered_set<unsigned int> m_unavailableEntries;
      LocalServiceLocatorDataStore m_cache;
      IO::OpenState m_openState;

      bool IsCached(const DirectoryEntry& entry);
      bool IsCached(unsigned int entry);
  };

  template<typename D>
  template<typename DF>
  CachedServiceLocatorDataStore<D>::CachedServiceLocatorDataStore(
      DF&& dataStore)
      : m_dataStore(std::forward<DF>(dataStore)) {
    try {
      auto directories = m_dataStore->LoadAllDirectories();
      for(auto& directory : directories) {
        m_cache.Store(directory);
      }
      for(auto& directory : directories) {
        auto parents = m_dataStore->LoadParents(directory);
        for(auto& parent : parents) {
          m_cache.Associate(directory, parent);
        }
      }
      auto accounts = m_dataStore->LoadAllAccounts();
      for(auto& account : accounts) {
        auto password = m_dataStore->LoadPassword(account);
        auto registrationTime = m_dataStore->LoadRegistrationTime(account);
        auto lastLoginTime = m_dataStore->LoadLastLoginTime(account);
        m_cache.Store(account, password, registrationTime, lastLoginTime);
        auto parents = m_dataStore->LoadParents(account);
        for(auto& parent : parents) {
          m_cache.Associate(account, parent);
        }
        auto permissions = m_dataStore->LoadAllPermissions(account);
        for(auto& permission : permissions) {
          m_cache.SetPermissions(account, std::get<0>(permission),
            std::get<1>(permission));
        }
      }
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename D>
  CachedServiceLocatorDataStore<D>::~CachedServiceLocatorDataStore() {
    Close();
  }

  template<typename D>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::LoadParents(
      const DirectoryEntry& entry) {
    if(IsCached(entry)) {
      return m_cache.LoadParents(entry);
    }
    return m_dataStore->LoadParents(entry);
  }

  template<typename D>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::LoadChildren(
      const DirectoryEntry& directory) {
    if(IsCached(directory)) {
      return m_cache.LoadChildren(directory);
    }
    return m_dataStore->LoadChildren(directory);
  }

  template<typename D>
  DirectoryEntry CachedServiceLocatorDataStore<D>::LoadDirectoryEntry(
      unsigned int id) {
    if(IsCached(id)) {
      return m_cache.LoadDirectoryEntry(id);
    }
    return m_dataStore->LoadDirectoryEntry(id);
  }

  template<typename D>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::
      LoadAllAccounts() {
    if(m_unavailableEntries.empty()) {
      return m_cache.LoadAllAccounts();
    }
    return m_dataStore->LoadAllAccounts();
  }

  template<typename D>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::
      LoadAllDirectories() {
    if(m_unavailableEntries.empty()) {
      return m_cache.LoadAllDirectories();
    }
    return m_dataStore->LoadAllDirectories();
  }

  template<typename D>
  DirectoryEntry CachedServiceLocatorDataStore<D>::LoadAccount(
      const std::string& name) {
    try {
      return m_cache.LoadAccount(name);
    } catch(const std::exception&) {
      return m_dataStore->LoadAccount(name);
    }
  }

  template<typename D>
  DirectoryEntry CachedServiceLocatorDataStore<D>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent, boost::posix_time::ptime registrationTime) {
    auto account = m_dataStore->MakeAccount(name, password, parent,
      registrationTime);
    try {
      auto cachedPassword = m_dataStore->LoadPassword(account);
      auto cachedRegistrationTime = m_dataStore->LoadRegistrationTime(account);
      auto cachedLoginTime = m_dataStore->LoadLastLoginTime(account);
      auto cachedParents = m_dataStore->LoadParents(account);
      auto cachedPermissions = m_dataStore->LoadAllPermissions(account);
      m_cache.Store(account, cachedPassword, cachedRegistrationTime,
        cachedLoginTime);
      for(auto& parent : cachedParents) {
        m_cache.Associate(account, parent);
      }
      for(auto& permissions : cachedPermissions) {
        m_cache.SetPermissions(account, std::get<0>(permissions),
          std::get<1>(permissions));
      }
    } catch(const std::exception&) {
      m_unavailableEntries.insert(account.m_id);
      m_cache.Delete(account);
    }
    return account;
  }

  template<typename D>
  DirectoryEntry CachedServiceLocatorDataStore<D>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    auto directory = m_dataStore->MakeDirectory(name, parent);
    try {
      auto cachedParents = m_dataStore->LoadParents(directory);
      auto cachedChildren = m_dataStore->LoadChildren(directory);
      m_cache.Store(directory);
      for(auto& parent : cachedParents) {
        m_cache.Associate(directory, parent);
      }
      for(auto& child : cachedChildren) {
        m_cache.Associate(child, directory);
      }
    } catch(const std::exception&) {
      m_unavailableEntries.insert(directory.m_id);
      m_cache.Delete(directory);
    }
    return directory;
  }

  template<typename D>
  void CachedServiceLocatorDataStore<D>::Delete(const DirectoryEntry& entry) {
    m_dataStore->Delete(entry);
    try {
      m_cache.Delete(entry);
      m_unavailableEntries.erase(entry.m_id);
    } catch(const std::exception&) {
      m_unavailableEntries.insert(entry.m_id);
    }
  }

  template<typename D>
  bool CachedServiceLocatorDataStore<D>::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(IsCached(entry) && IsCached(parent)) {
      if(m_dataStore->Associate(entry, parent)) {
        m_cache.Associate(entry, parent);
        return true;
      }
      return false;
    }
    m_unavailableEntries.insert(entry.m_id);
    m_unavailableEntries.insert(parent.m_id);
    return m_dataStore->Associate(entry, parent);
  }

  template<typename D>
  bool CachedServiceLocatorDataStore<D>::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(IsCached(entry) && IsCached(parent)) {
      if(m_dataStore->Detach(entry, parent)) {
        m_cache.Detach(entry, parent);
        return true;
      }
      return false;
    }
    m_unavailableEntries.insert(entry.m_id);
    m_unavailableEntries.insert(parent.m_id);
    return m_dataStore->Detach(entry, parent);
  }

  template<typename D>
  std::string CachedServiceLocatorDataStore<D>::LoadPassword(
      const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadPassword(account);
    }
    return m_dataStore->LoadPassword(account);
  }

  template<typename D>
  void CachedServiceLocatorDataStore<D>::SetPassword(
      const DirectoryEntry& account, const std::string& password) {
    if(IsCached(account)) {
      m_dataStore->SetPassword(account, password);
      m_cache.SetPassword(account, password);
    } else {
      m_dataStore->SetPassword(account, password);
    }
  }

  template<typename D>
  Permissions CachedServiceLocatorDataStore<D>::LoadPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    if(IsCached(source) && IsCached(target)) {
      return m_cache.LoadPermissions(source, target);
    }
    return m_dataStore->LoadPermissions(source, target);
  }

  template<typename D>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      CachedServiceLocatorDataStore<D>::LoadAllPermissions(
      const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadAllPermissions(account);
    }
    return m_dataStore->LoadAllPermissions(account);
  }

  template<typename D>
  void CachedServiceLocatorDataStore<D>::SetPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    if(IsCached(source) && IsCached(target)) {
      m_dataStore->SetPermissions(source, target, permissions);
      m_cache.SetPermissions(source, target, permissions);
    } else {
      m_unavailableEntries.insert(source.m_id);
      m_unavailableEntries.insert(target.m_id);
      m_dataStore->SetPermissions(source, target, permissions);
    }
  }

  template<typename D>
  boost::posix_time::ptime CachedServiceLocatorDataStore<D>::
      LoadRegistrationTime(const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadRegistrationTime(account);
    }
    return m_dataStore->LoadRegistrationTime(account);
  }

  template<typename D>
  boost::posix_time::ptime CachedServiceLocatorDataStore<D>::LoadLastLoginTime(
      const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadLastLoginTime(account);
    }
    return m_dataStore->LoadLastLoginTime(account);
  }

  template<typename D>
  void CachedServiceLocatorDataStore<D>::StoreLastLoginTime(
      const DirectoryEntry& account, boost::posix_time::ptime loginTime) {
    if(IsCached(account)) {
      m_dataStore->StoreLastLoginTime(account, loginTime);
      m_cache.StoreLastLoginTime(account, loginTime);
    } else {
      m_dataStore->StoreLastLoginTime(account, loginTime);
    }
  }

  template<typename D>
  void CachedServiceLocatorDataStore<D>::Rename(const DirectoryEntry& entry,
      const std::string& name) {
    m_cache.Rename(entry, name);
    m_dataStore->Rename(entry, name);
  }

  template<typename D>
  DirectoryEntry CachedServiceLocatorDataStore<D>::Validate(
      const DirectoryEntry& entry) {
    if(IsCached(entry)) {
      return m_cache.Validate(entry);
    }
    return m_dataStore->Validate(entry);
  }

  template<typename D>
  void CachedServiceLocatorDataStore<D>::WithTransaction(
      const std::function<void ()>& transaction) {
    m_dataStore->WithTransaction([&] {
      m_cache.WithTransaction(transaction);
    });
  }

  template<typename D>
  void CachedServiceLocatorDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_cache.Close();
    m_openState.Close();
  }

  template<typename D>
  bool CachedServiceLocatorDataStore<D>::IsCached(const DirectoryEntry& entry) {
    return IsCached(entry.m_id);
  }

  template<typename D>
  bool CachedServiceLocatorDataStore<D>::IsCached(unsigned int id) {
    return m_unavailableEntries.find(id) == m_unavailableEntries.end();
  }
}

#endif
