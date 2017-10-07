#ifndef BEAM_CACHEDSERVICELOCATORDATASTORE_HPP
#define BEAM_CACHEDSERVICELOCATORDATASTORE_HPP
#include <unordered_set>
#include "Beam/IO/OpenState.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class CachedServiceLocatorDataStore
      \brief Caches all data from a ServiceLocatorDataStore.
      \tparam DataStoreType The type of ServiceLocatorDataStore to cache.
   */
  template<typename DataStoreType>
  class CachedServiceLocatorDataStore : public ServiceLocatorDataStore {
    public:

      //! The type of ServiceLocatorDataStore to cache.
      using DataStore = GetTryDereferenceType<DataStoreType>;

      //! Constructs a CachedServiceLocatorDataStore.
      /*!
        \param dataStore The ServiceLocatorDataStore to cache.
      */
      template<typename DataStoreForward>
      CachedServiceLocatorDataStore(DataStoreForward&& dataStore);

      virtual ~CachedServiceLocatorDataStore() override;

      virtual std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override;

      virtual std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& directory) override;

      virtual DirectoryEntry LoadDirectoryEntry(unsigned int id) override;

      virtual std::vector<DirectoryEntry> LoadAllAccounts() override;

      virtual std::vector<DirectoryEntry> LoadAllDirectories() override;

      virtual DirectoryEntry LoadAccount(const std::string& name) override;

      virtual DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent,
        const boost::posix_time::ptime& registrationTime) override;

      virtual DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) override;

      virtual void Delete(const DirectoryEntry& entry) override;

      virtual bool Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      virtual bool Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      virtual std::string LoadPassword(const DirectoryEntry& account) override;

      virtual void SetPassword(const DirectoryEntry& account,
        const std::string& password) override;

      virtual Permissions LoadPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target) override;

      virtual std::vector<std::tuple<DirectoryEntry, Permissions>>
        LoadAllPermissions(const DirectoryEntry& account);

      virtual void SetPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override;

      virtual boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) override;

      virtual boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) override;

      virtual void StoreLastLoginTime(const DirectoryEntry& account,
        const boost::posix_time::ptime& loginTime) override;

      virtual void Rename(const DirectoryEntry& entry,
        const std::string& name) override;

      virtual DirectoryEntry Validate(const DirectoryEntry& entry) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      GetOptionalLocalPtr<DataStoreType> m_dataStore;
      std::unordered_set<unsigned int> m_unavailableEntries;
      LocalServiceLocatorDataStore m_cache;
      IO::OpenState m_openState;

      void Shutdown();
      bool IsCached(const DirectoryEntry& entry);
      bool IsCached(unsigned int entry);
  };

  template<typename DataStoreType>
  template<typename DataStoreForward>
  CachedServiceLocatorDataStore<DataStoreType>::CachedServiceLocatorDataStore(
      DataStoreForward&& dataStore)
      : m_dataStore{std::forward<DataStoreForward>(dataStore)} {}

  template<typename DataStoreType>
  CachedServiceLocatorDataStore<DataStoreType>::
      ~CachedServiceLocatorDataStore() {
    Close();
  }

  template<typename DataStoreType>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<DataStoreType>::
      LoadParents(const DirectoryEntry& entry) {
    if(IsCached(entry)) {
      return m_cache.LoadParents(entry);
    }
    return m_dataStore->LoadParents(entry);
  }

  template<typename DataStoreType>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<DataStoreType>::
      LoadChildren(const DirectoryEntry& directory) {
    if(IsCached(directory)) {
      return m_cache.LoadChildren(directory);
    }
    return m_dataStore->LoadChildren(directory);
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::
      LoadDirectoryEntry(unsigned int id) {
    if(IsCached(id)) {
      return m_cache.LoadDirectoryEntry(id);
    }
    return m_dataStore->LoadDirectoryEntry(id);
  }

  template<typename DataStoreType>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<DataStoreType>::
      LoadAllAccounts() {
    if(m_unavailableEntries.empty()) {
      return m_cache.LoadAllAccounts();
    }
    return m_dataStore->LoadAllAccounts();
  }

  template<typename DataStoreType>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<DataStoreType>::
      LoadAllDirectories() {
    if(m_unavailableEntries.empty()) {
      return m_cache.LoadAllDirectories();
    }
    return m_dataStore->LoadAllDirectories();
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::LoadAccount(
      const std::string& name) {
    try {
      return m_cache.LoadAccount(name);
    } catch(const std::exception&) {
      return m_dataStore->LoadAccount(name);
    }
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      const boost::posix_time::ptime& registrationTime) {
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

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::MakeDirectory(
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

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::Delete(
      const DirectoryEntry& entry) {
    m_dataStore->Delete(entry);
    try {
      m_cache.Delete(entry);
      m_unavailableEntries.erase(entry.m_id);
    } catch(const std::exception&) {
      m_unavailableEntries.insert(entry.m_id);
    }
  }

  template<typename DataStoreType>
  bool CachedServiceLocatorDataStore<DataStoreType>::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
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

  template<typename DataStoreType>
  bool CachedServiceLocatorDataStore<DataStoreType>::Detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
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

  template<typename DataStoreType>
  std::string CachedServiceLocatorDataStore<DataStoreType>::LoadPassword(
      const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadPassword(account);
    }
    return m_dataStore->LoadPassword(account);
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::SetPassword(
      const DirectoryEntry& account, const std::string& password) {
    if(IsCached(account)) {
      m_dataStore->SetPassword(account, password);
      m_cache.SetPassword(account, password);
    } else {
      m_dataStore->SetPassword(account, password);
    }
  }

  template<typename DataStoreType>
  Permissions CachedServiceLocatorDataStore<DataStoreType>::LoadPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    if(IsCached(source) && IsCached(target)) {
      return m_cache.LoadPermissions(source, target);
    }
    return m_dataStore->LoadPermissions(source, target);
  }

  template<typename DataStoreType>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      CachedServiceLocatorDataStore<DataStoreType>::LoadAllPermissions(
      const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadAllPermissions(account);
    }
    return m_dataStore->LoadAllPermissions(account);
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::SetPermissions(
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

  template<typename DataStoreType>
  boost::posix_time::ptime CachedServiceLocatorDataStore<DataStoreType>::
      LoadRegistrationTime(const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadRegistrationTime(account);
    }
    return m_dataStore->LoadRegistrationTime(account);
  }

  template<typename DataStoreType>
  boost::posix_time::ptime CachedServiceLocatorDataStore<DataStoreType>::
      LoadLastLoginTime(const DirectoryEntry& account) {
    if(IsCached(account)) {
      return m_cache.LoadLastLoginTime(account);
    }
    return m_dataStore->LoadLastLoginTime(account);
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::StoreLastLoginTime(
      const DirectoryEntry& account,
      const boost::posix_time::ptime& loginTime) {
    if(IsCached(account)) {
      m_dataStore->StoreLastLoginTime(account, loginTime);
      m_cache.StoreLastLoginTime(account, loginTime);
    } else {
      m_dataStore->StoreLastLoginTime(account, loginTime);
    }
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    m_cache.Rename(entry, name);
    m_dataStore->Rename(entry, name);
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::Validate(
      const DirectoryEntry& entry) {
    if(IsCached(entry)) {
      return m_cache.Validate(entry);
    }
    return m_dataStore->Validate(entry);
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::WithTransaction(
      const std::function<void ()>& transaction) {
    m_dataStore->WithTransaction(
      [&] {
        m_cache.WithTransaction(transaction);
      });
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_cache.Open();
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
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::Shutdown() {
    m_dataStore->Close();
    m_cache.Close();
    m_openState.SetClosed();
  }

  template<typename DataStoreType>
  bool CachedServiceLocatorDataStore<DataStoreType>::IsCached(
      const DirectoryEntry& entry) {
    return IsCached(entry.m_id);
  }

  template<typename DataStoreType>
  bool CachedServiceLocatorDataStore<DataStoreType>::IsCached(unsigned int id) {
    if(m_unavailableEntries.find(id) == m_unavailableEntries.end()) {
      return true;
    }
    return false;
  }
}
}

#endif
