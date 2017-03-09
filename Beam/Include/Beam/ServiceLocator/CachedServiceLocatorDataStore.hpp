#ifndef BEAM_CACHEDSERVICELOCATORDATASTORE_HPP
#define BEAM_CACHEDSERVICELOCATORDATASTORE_HPP
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

      virtual ~CachedServiceLocatorDataStore();

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

      virtual DirectoryEntry Validate(const DirectoryEntry& entry) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      GetOptionalLocalPtr<DataStoreType> m_dataStore;
      LocalServiceLocatorDataStore m_cache;
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
    return m_cache.LoadParents(entry);
  }

  template<typename DataStoreType>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<DataStoreType>::
      LoadChildren(const DirectoryEntry& directory) {
    return m_cache.LoadChildren(directory);
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::
      LoadDirectoryEntry(unsigned int id) {
    return m_cache.LoadDirectoryEntry(id);
  }

  template<typename DataStoreType>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<DataStoreType>::
      LoadAllAccounts() {
    return m_cache.LoadAllAccounts();
  }

  template<typename DataStoreType>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<DataStoreType>::
      LoadAllDirectories() {
    return m_cache.LoadAllDirectories();
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::LoadAccount(
      const std::string& name) {
    return m_cache.LoadAccount(name);
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      const boost::posix_time::ptime& registrationTime) {
    auto account = m_dataStore->MakeAccount(
      name, password, parent, registrationTime);
    m_cache.Store(account, password, registrationTime,
      boost::posix_time::neg_infin);
    m_cache.Associate(account, parent);
    return account;
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    auto directory = m_dataStore->MakeDirectory(name, parent);
    m_cache.Store(directory);
    m_cache.Associate(directory, parent);
    return directory;
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::Delete(
      const DirectoryEntry& entry) {
    m_dataStore->Delete(entry);
    m_cache.Delete(entry);
  }

  template<typename DataStoreType>
  bool CachedServiceLocatorDataStore<DataStoreType>::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    if(m_dataStore->Associate(entry, parent)) {
      return m_cache.Associate(entry, parent);
    }
    return false;
  }

  template<typename DataStoreType>
  bool CachedServiceLocatorDataStore<DataStoreType>::Detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    if(m_dataStore->Detach(entry, parent)) {
      return m_cache.Detach(entry, parent);
    }
    return false;
  }

  template<typename DataStoreType>
  std::string CachedServiceLocatorDataStore<DataStoreType>::LoadPassword(
      const DirectoryEntry& account) {
    return m_cache.LoadPassword(account);
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::SetPassword(
      const DirectoryEntry& account, const std::string& password) {
    m_dataStore->SetPassword(account, password);
    m_cache.SetPassword(account, password);
  }

  template<typename DataStoreType>
  Permissions CachedServiceLocatorDataStore<DataStoreType>::LoadPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    return m_cache.LoadPermissions(source, target);
  }

  template<typename DataStoreType>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      CachedServiceLocatorDataStore<DataStoreType>::LoadAllPermissions(
      const DirectoryEntry& account) {
    return m_cache.LoadAllPermissions(account);
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::SetPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_dataStore->SetPermissions(source, target, permissions);
    m_cache.SetPermissions(source, target, permissions);
  }

  template<typename DataStoreType>
  boost::posix_time::ptime CachedServiceLocatorDataStore<DataStoreType>::
      LoadRegistrationTime(const DirectoryEntry& account) {
    return m_cache.LoadRegistrationTime(account);
  }

  template<typename DataStoreType>
  boost::posix_time::ptime CachedServiceLocatorDataStore<DataStoreType>::
      LoadLastLoginTime(const DirectoryEntry& account) {
    return m_cache.LoadLastLoginTime(account);
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::StoreLastLoginTime(
      const DirectoryEntry& account,
      const boost::posix_time::ptime& loginTime) {
    m_dataStore->StoreLastLoginTime(account, loginTime);
    m_cache.StoreLastLoginTime(account, loginTime);
  }

  template<typename DataStoreType>
  DirectoryEntry CachedServiceLocatorDataStore<DataStoreType>::Validate(
      const DirectoryEntry& entry) {
    return m_cache.Validate(entry);
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
  }

  template<typename DataStoreType>
  void CachedServiceLocatorDataStore<DataStoreType>::Close() {
    m_dataStore->Close();
    m_cache.Close();
  }
}
}

#endif
