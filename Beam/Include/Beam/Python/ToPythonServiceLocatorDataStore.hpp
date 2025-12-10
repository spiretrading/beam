#ifndef BEAM_TO_PYTHON_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_TO_PYTHON_SERVICE_LOCATOR_DATA_STORE_HPP
#include <utility>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"

namespace Beam::Python {

  /**
   * Wraps a ServiceLocatorDataStore class for use within Python.
   * @tparam D The type of ServiceLocatorDataStore to wrap.
   */
  template<IsServiceLocatorDataStore D>
  class ToPythonServiceLocatorDataStore {
    public:

      /** The type of ServiceLocatorDataStore to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonServiceLocatorDataStore in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonServiceLocatorDataStore(Args&&... args);

      ~ToPythonServiceLocatorDataStore();

      /** Returns a reference to the underlying data store. */
      DataStore& get();

      /** Returns a reference to the underlying data store. */
      const DataStore& get() const;

      std::vector<DirectoryEntry> load_parents(const DirectoryEntry& entry);
      std::vector<DirectoryEntry>
        load_children(const DirectoryEntry& directory);
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
      std::vector<std::tuple<DirectoryEntry, Permissions>>
        load_all_permissions(const DirectoryEntry& account);
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
      boost::optional<DataStore> m_data_store;

      ToPythonServiceLocatorDataStore(
        const ToPythonServiceLocatorDataStore&) = delete;
      ToPythonServiceLocatorDataStore& operator =(
        const ToPythonServiceLocatorDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonServiceLocatorDataStore(DataStore&&) ->
    ToPythonServiceLocatorDataStore<std::remove_cvref_t<DataStore>>;

  template<IsServiceLocatorDataStore D>
  template<typename... Args>
  ToPythonServiceLocatorDataStore<D>::ToPythonServiceLocatorDataStore(
    Args&&... args)
    : m_data_store((GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsServiceLocatorDataStore D>
  ToPythonServiceLocatorDataStore<D>::~ToPythonServiceLocatorDataStore() {
    auto release = GilRelease();
    m_data_store.reset();
  }

  template<IsServiceLocatorDataStore D>
  typename ToPythonServiceLocatorDataStore<D>::DataStore&
      ToPythonServiceLocatorDataStore<D>::get() {
    return *m_data_store;
  }

  template<IsServiceLocatorDataStore D>
  const typename ToPythonServiceLocatorDataStore<D>::DataStore&
      ToPythonServiceLocatorDataStore<D>::get() const {
    return *m_data_store;
  }

  template<IsServiceLocatorDataStore D>
  std::vector<DirectoryEntry> ToPythonServiceLocatorDataStore<D>::load_parents(
      const DirectoryEntry& entry) {
    auto release = GilRelease();
    return m_data_store->load_parents(entry);
  }

  template<IsServiceLocatorDataStore D>
  std::vector<DirectoryEntry> ToPythonServiceLocatorDataStore<D>::load_children(
      const DirectoryEntry& directory) {
    auto release = GilRelease();
    return m_data_store->load_children(directory);
  }

  template<IsServiceLocatorDataStore D>
  DirectoryEntry ToPythonServiceLocatorDataStore<D>::load_directory_entry(
      unsigned int id) {
    auto release = GilRelease();
    return m_data_store->load_directory_entry(id);
  }

  template<IsServiceLocatorDataStore D>
  std::vector<DirectoryEntry>
      ToPythonServiceLocatorDataStore<D>::load_all_accounts() {
    auto release = GilRelease();
    return m_data_store->load_all_accounts();
  }

  template<IsServiceLocatorDataStore D>
  std::vector<DirectoryEntry>
      ToPythonServiceLocatorDataStore<D>::load_all_directories() {
    auto release = GilRelease();
    return m_data_store->load_all_directories();
  }

  template<IsServiceLocatorDataStore D>
  DirectoryEntry ToPythonServiceLocatorDataStore<D>::load_account(
      const std::string& name) {
    auto release = GilRelease();
    return m_data_store->load_account(name);
  }

  template<IsServiceLocatorDataStore D>
  DirectoryEntry ToPythonServiceLocatorDataStore<D>::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      boost::posix_time::ptime registration_time) {
    auto release = GilRelease();
    return m_data_store->make_account(
      name, password, parent, registration_time);
  }

  template<IsServiceLocatorDataStore D>
  DirectoryEntry ToPythonServiceLocatorDataStore<D>::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    auto release = GilRelease();
    return m_data_store->make_directory(name, parent);
  }

  template<IsServiceLocatorDataStore D>
  void ToPythonServiceLocatorDataStore<D>::remove(
      const DirectoryEntry& entry) {
    auto release = GilRelease();
    m_data_store->remove(entry);
  }

  template<IsServiceLocatorDataStore D>
  bool ToPythonServiceLocatorDataStore<D>::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto release = GilRelease();
    return m_data_store->associate(entry, parent);
  }

  template<IsServiceLocatorDataStore D>
  bool ToPythonServiceLocatorDataStore<D>::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto release = GilRelease();
    return m_data_store->detach(entry, parent);
  }

  template<IsServiceLocatorDataStore D>
  std::string ToPythonServiceLocatorDataStore<D>::load_password(
      const DirectoryEntry& account) {
    auto release = GilRelease();
    return m_data_store->load_password(account);
  }

  template<IsServiceLocatorDataStore D>
  void ToPythonServiceLocatorDataStore<D>::set_password(
      const DirectoryEntry& account, const std::string& password) {
    auto release = GilRelease();
    m_data_store->set_password(account, password);
  }

  template<IsServiceLocatorDataStore D>
  Permissions ToPythonServiceLocatorDataStore<D>::load_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    auto release = GilRelease();
    return m_data_store->load_permissions(source, target);
  }

  template<IsServiceLocatorDataStore D>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      ToPythonServiceLocatorDataStore<D>::load_all_permissions(
        const DirectoryEntry& account) {
    auto release = GilRelease();
    return m_data_store->load_all_permissions(account);
  }

  template<IsServiceLocatorDataStore D>
  void ToPythonServiceLocatorDataStore<D>::set_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto release = GilRelease();
    m_data_store->set_permissions(source, target, permissions);
  }

  template<IsServiceLocatorDataStore D>
  boost::posix_time::ptime
      ToPythonServiceLocatorDataStore<D>::load_registration_time(
        const DirectoryEntry& account) {
    auto release = GilRelease();
    return m_data_store->load_registration_time(account);
  }

  template<IsServiceLocatorDataStore D>
  boost::posix_time::ptime
      ToPythonServiceLocatorDataStore<D>::load_last_login_time(
        const DirectoryEntry& account) {
    auto release = GilRelease();
    return m_data_store->load_last_login_time(account);
  }

  template<IsServiceLocatorDataStore D>
  void ToPythonServiceLocatorDataStore<D>::store_last_login_time(
      const DirectoryEntry& account, boost::posix_time::ptime login_time) {
    auto release = GilRelease();
    m_data_store->store_last_login_time(account, login_time);
  }

  template<IsServiceLocatorDataStore D>
  void ToPythonServiceLocatorDataStore<D>::rename(
      const DirectoryEntry& entry, const std::string& name) {
    auto release = GilRelease();
    m_data_store->rename(entry, name);
  }

  template<IsServiceLocatorDataStore D>
  template<std::invocable<> F>
  decltype(auto) ToPythonServiceLocatorDataStore<D>::with_transaction(
      F&& transaction) {
    auto release = GilRelease();
    return m_data_store->with_transaction(std::forward<F>(transaction));
  }

  template<IsServiceLocatorDataStore D>
  void ToPythonServiceLocatorDataStore<D>::close() {
    auto release = GilRelease();
    m_data_store->close();
  }
}

#endif
