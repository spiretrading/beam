#ifndef BEAM_CACHED_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_CACHED_SERVICE_LOCATOR_DATA_STORE_HPP
#include <unordered_set>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Caches all data from a ServiceLocatorDataStore.
   * @tparam D The type of ServiceLocatorDataStore to cache.
   */
  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  class CachedServiceLocatorDataStore {
    public:

      /** The type of ServiceLocatorDataStore to cache. */
      using DataStore = dereference_t<D>;

      /**
       * Constructs a CachedServiceLocatorDataStore.
       * @param data_store The ServiceLocatorDataStore to cache.
       */
      template<Initializes<D> DF>
      explicit CachedServiceLocatorDataStore(DF&& data_store);

      ~CachedServiceLocatorDataStore();

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
      local_ptr_t<D> m_data_store;
      std::unordered_set<unsigned int> m_unavailable_entries;
      LocalServiceLocatorDataStore m_cache;
      OpenState m_open_state;

      bool is_cached(const DirectoryEntry& entry);
      bool is_cached(unsigned int id);
  };

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  template<Initializes<D> DF>
  CachedServiceLocatorDataStore<D>::CachedServiceLocatorDataStore(
      DF&& data_store)
      : m_data_store(std::forward<DF>(data_store)) {
    try {
      auto directories = m_data_store->load_all_directories();
      for(auto& directory : directories) {
        m_cache.store(directory);
      }
      for(auto& directory : directories) {
        auto parents = m_data_store->load_parents(directory);
        for(auto& parent : parents) {
          m_cache.associate(directory, parent);
        }
      }
      auto accounts = m_data_store->load_all_accounts();
      for(auto& account : accounts) {
        auto password = m_data_store->load_password(account);
        auto registration_time = m_data_store->load_registration_time(account);
        auto last_login_time = m_data_store->load_last_login_time(account);
        m_cache.store(account, password, registration_time, last_login_time);
        auto parents = m_data_store->load_parents(account);
        for(auto& parent : parents) {
          m_cache.associate(account, parent);
        }
        auto permissions = m_data_store->load_all_permissions(account);
        for(auto& permission : permissions) {
          m_cache.set_permissions(account, std::get<0>(permission),
            std::get<1>(permission));
        }
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  CachedServiceLocatorDataStore<D>::~CachedServiceLocatorDataStore() {
    close();
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::load_parents(
      const DirectoryEntry& entry) {
    if(is_cached(entry)) {
      return m_cache.load_parents(entry);
    }
    return m_data_store->load_parents(entry);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::load_children(
      const DirectoryEntry& directory) {
    if(is_cached(directory)) {
      return m_cache.load_children(directory);
    }
    return m_data_store->load_children(directory);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry CachedServiceLocatorDataStore<D>::load_directory_entry(
      unsigned int id) {
    if(is_cached(id)) {
      return m_cache.load_directory_entry(id);
    }
    return m_data_store->load_directory_entry(id);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::
      load_all_accounts() {
    if(m_unavailable_entries.empty()) {
      return m_cache.load_all_accounts();
    }
    return m_data_store->load_all_accounts();
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> CachedServiceLocatorDataStore<D>::
      load_all_directories() {
    if(m_unavailable_entries.empty()) {
      return m_cache.load_all_directories();
    }
    return m_data_store->load_all_directories();
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry CachedServiceLocatorDataStore<D>::load_account(
      const std::string& name) {
    try {
      return m_cache.load_account(name);
    } catch(const std::exception&) {
      return m_data_store->load_account(name);
    }
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry CachedServiceLocatorDataStore<D>::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      boost::posix_time::ptime registration_time) {
    auto account = m_data_store->make_account(name, password, parent,
      registration_time);
    try {
      auto cached_password = m_data_store->load_password(account);
      auto cached_registration_time =
        m_data_store->load_registration_time(account);
      auto cached_login_time = m_data_store->load_last_login_time(account);
      auto cached_parents = m_data_store->load_parents(account);
      auto cached_permissions = m_data_store->load_all_permissions(account);
      m_cache.store(account, cached_password, cached_registration_time,
        cached_login_time);
      for(auto& parent : cached_parents) {
        m_cache.associate(account, parent);
      }
      for(auto& permissions : cached_permissions) {
        m_cache.set_permissions(account, std::get<0>(permissions),
          std::get<1>(permissions));
      }
    } catch(const std::exception&) {
      m_unavailable_entries.insert(account.m_id);
      m_cache.remove(account);
    }
    return account;
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry CachedServiceLocatorDataStore<D>::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    auto directory = m_data_store->make_directory(name, parent);
    try {
      auto cached_parents = m_data_store->load_parents(directory);
      auto cached_children = m_data_store->load_children(directory);
      m_cache.store(directory);
      for(auto& parent : cached_parents) {
        m_cache.associate(directory, parent);
      }
      for(auto& child : cached_children) {
        m_cache.associate(child, directory);
      }
    } catch(const std::exception&) {
      m_unavailable_entries.insert(directory.m_id);
      m_cache.remove(directory);
    }
    return directory;
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  void CachedServiceLocatorDataStore<D>::remove(const DirectoryEntry& entry) {
    m_data_store->remove(entry);
    try {
      m_cache.remove(entry);
      m_unavailable_entries.erase(entry.m_id);
    } catch(const std::exception&) {
      m_unavailable_entries.insert(entry.m_id);
    }
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  bool CachedServiceLocatorDataStore<D>::associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(is_cached(entry) && is_cached(parent)) {
      if(m_data_store->associate(entry, parent)) {
        m_cache.associate(entry, parent);
        return true;
      }
      return false;
    }
    m_unavailable_entries.insert(entry.m_id);
    m_unavailable_entries.insert(parent.m_id);
    return m_data_store->associate(entry, parent);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  bool CachedServiceLocatorDataStore<D>::detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(is_cached(entry) && is_cached(parent)) {
      if(m_data_store->detach(entry, parent)) {
        m_cache.detach(entry, parent);
        return true;
      }
      return false;
    }
    m_unavailable_entries.insert(entry.m_id);
    m_unavailable_entries.insert(parent.m_id);
    return m_data_store->detach(entry, parent);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  std::string CachedServiceLocatorDataStore<D>::load_password(
      const DirectoryEntry& account) {
    if(is_cached(account)) {
      return m_cache.load_password(account);
    }
    return m_data_store->load_password(account);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  void CachedServiceLocatorDataStore<D>::set_password(
      const DirectoryEntry& account, const std::string& password) {
    if(is_cached(account)) {
      m_data_store->set_password(account, password);
      m_cache.set_password(account, password);
    } else {
      m_data_store->set_password(account, password);
    }
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  Permissions CachedServiceLocatorDataStore<D>::load_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    if(is_cached(source) && is_cached(target)) {
      return m_cache.load_permissions(source, target);
    }
    return m_data_store->load_permissions(source, target);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      CachedServiceLocatorDataStore<D>::load_all_permissions(
      const DirectoryEntry& account) {
    if(is_cached(account)) {
      return m_cache.load_all_permissions(account);
    }
    return m_data_store->load_all_permissions(account);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  void CachedServiceLocatorDataStore<D>::set_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    if(is_cached(source) && is_cached(target)) {
      m_data_store->set_permissions(source, target, permissions);
      m_cache.set_permissions(source, target, permissions);
    } else {
      m_unavailable_entries.insert(source.m_id);
      m_unavailable_entries.insert(target.m_id);
      m_data_store->set_permissions(source, target, permissions);
    }
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  boost::posix_time::ptime CachedServiceLocatorDataStore<D>::
      load_registration_time(const DirectoryEntry& account) {
    if(is_cached(account)) {
      return m_cache.load_registration_time(account);
    }
    return m_data_store->load_registration_time(account);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  boost::posix_time::ptime CachedServiceLocatorDataStore<D>::
      load_last_login_time(const DirectoryEntry& account) {
    if(is_cached(account)) {
      return m_cache.load_last_login_time(account);
    }
    return m_data_store->load_last_login_time(account);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  void CachedServiceLocatorDataStore<D>::store_last_login_time(
      const DirectoryEntry& account, boost::posix_time::ptime login_time) {
    if(is_cached(account)) {
      m_data_store->store_last_login_time(account, login_time);
      m_cache.store_last_login_time(account, login_time);
    } else {
      m_data_store->store_last_login_time(account, login_time);
    }
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  void CachedServiceLocatorDataStore<D>::rename(
      const DirectoryEntry& entry, const std::string& name) {
    m_cache.rename(entry, name);
    m_data_store->rename(entry, name);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  template<std::invocable<> F>
  decltype(auto) CachedServiceLocatorDataStore<D>::with_transaction(
      F&& transaction) {
    return m_data_store->with_transaction([&] {
      return m_cache.with_transaction(std::forward<F>(transaction));
    });
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  void CachedServiceLocatorDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_data_store->close();
    m_cache.close();
    m_open_state.close();
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  bool CachedServiceLocatorDataStore<D>::is_cached(
      const DirectoryEntry& entry) {
    return is_cached(entry.m_id);
  }

  template<typename D> requires IsServiceLocatorDataStore<dereference_t<D>>
  bool CachedServiceLocatorDataStore<D>::is_cached(unsigned int id) {
    return m_unavailable_entries.find(id) == m_unavailable_entries.end();
  }
}

#endif
