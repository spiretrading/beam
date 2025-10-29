#ifndef BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#include <utility>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"

namespace Beam::Python {

  /**
   * Wraps a ServiceLocatorClient class for use within Python.
   * @tparam C The type of ServiceLocatorClient to wrap.
   */
  template<IsServiceLocatorClient C>
  class ToPythonServiceLocatorClient {
    public:

      /** The type of ServiceLocatorClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonServiceLocatorClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonServiceLocatorClient(Args&&... args);

      ~ToPythonServiceLocatorClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      /** Returns a reference to the underlying client. */
      Client& operator *();

      /** Returns a reference to the underlying client. */
      const Client& operator *() const;

      /** Returns a pointer to the underlying client. */
      Client* operator ->();

      /** Returns a pointer to the underlying client. */
      const Client* operator ->() const;

      DirectoryEntry get_account() const;
      std::string get_session_id() const;
      std::string get_encrypted_session_id(unsigned int key) const;
      DirectoryEntry authenticate_account(
        const std::string& username, const std::string& password);
      DirectoryEntry authenticate_session(
        const std::string& session_id, unsigned int key);
      std::vector<ServiceEntry> locate(const std::string& name);
      ServiceEntry add(const std::string& name, const JsonObject& properties);
      void remove(const ServiceEntry& service);
      std::vector<DirectoryEntry> load_all_accounts();
      boost::optional<DirectoryEntry> find_account(const std::string& name);
      DirectoryEntry make_account(const std::string& name,
        const std::string& password, const DirectoryEntry& parent);
      DirectoryEntry make_directory(
        const std::string& name, const DirectoryEntry& parent);
      void store_password(
        const DirectoryEntry& account, const std::string& password);
      void monitor(ScopedQueueWriter<AccountUpdate> queue);
      DirectoryEntry load_directory_entry(
        const DirectoryEntry& root, const std::string& path);
      DirectoryEntry load_directory_entry(unsigned int id);
      std::vector<DirectoryEntry> load_parents(const DirectoryEntry& entry);
      std::vector<DirectoryEntry> load_children(const DirectoryEntry& entry);
      void remove(const DirectoryEntry& entry);
      void associate(
        const DirectoryEntry& entry, const DirectoryEntry& parent);
      void detach(const DirectoryEntry& entry, const DirectoryEntry& parent);
      bool has_permissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions);
      void store(const DirectoryEntry& source, const DirectoryEntry& target,
        Permissions permissions);
      boost::posix_time::ptime load_registration_time(
        const DirectoryEntry& account);
      boost::posix_time::ptime load_last_login_time(
        const DirectoryEntry& account);
      DirectoryEntry rename(
        const DirectoryEntry& entry, const std::string& name);
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonServiceLocatorClient(const ToPythonServiceLocatorClient&) =
        delete;
      ToPythonServiceLocatorClient& operator =(
        const ToPythonServiceLocatorClient&) = delete;
  };

  template<typename Client>
  ToPythonServiceLocatorClient(Client&&) ->
    ToPythonServiceLocatorClient<std::remove_cvref_t<Client>>;

  template<IsServiceLocatorClient C>
  template<typename... Args>
  ToPythonServiceLocatorClient<C>::ToPythonServiceLocatorClient(Args&&... args)
    : m_client(
        (GilRelease(), boost::in_place_init), std::forward<Args>(args)...) {}

  template<IsServiceLocatorClient C>
  ToPythonServiceLocatorClient<C>::~ToPythonServiceLocatorClient() {
    auto release = GilRelease();
    m_client.reset();
  }

  template<IsServiceLocatorClient C>
  typename ToPythonServiceLocatorClient<C>::Client&
      ToPythonServiceLocatorClient<C>::get() {
    return *m_client;
  }

  template<IsServiceLocatorClient C>
  const typename ToPythonServiceLocatorClient<C>::Client&
      ToPythonServiceLocatorClient<C>::get() const {
    return *m_client;
  }

  template<IsServiceLocatorClient C>
  typename ToPythonServiceLocatorClient<C>::Client&
      ToPythonServiceLocatorClient<C>::operator *() {
    return *m_client;
  }

  template<IsServiceLocatorClient C>
  const typename ToPythonServiceLocatorClient<C>::Client&
      ToPythonServiceLocatorClient<C>::operator *() const {
    return *m_client;
  }

  template<IsServiceLocatorClient C>
  typename ToPythonServiceLocatorClient<C>::Client*
      ToPythonServiceLocatorClient<C>::operator ->() {
    return m_client.get_ptr();
  }

  template<IsServiceLocatorClient C>
  const typename ToPythonServiceLocatorClient<C>::Client*
      ToPythonServiceLocatorClient<C>::operator ->() const {
    return m_client.get_ptr();
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::get_account() const {
    auto release = GilRelease();
    return m_client->get_account();
  }

  template<IsServiceLocatorClient C>
  std::string ToPythonServiceLocatorClient<C>::get_session_id() const {
    auto release = GilRelease();
    return m_client->get_session_id();
  }

  template<IsServiceLocatorClient C>
  std::string ToPythonServiceLocatorClient<C>::get_encrypted_session_id(
      unsigned int key) const {
    auto release = GilRelease();
    return m_client->get_encrypted_session_id(key);
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::authenticate_account(
      const std::string& username, const std::string& password) {
    auto release = GilRelease();
    return m_client->authenticate_account(username, password);
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::authenticate_session(
      const std::string& session_id, unsigned int key) {
    auto release = GilRelease();
    return m_client->authenticate_session(session_id, key);
  }

  template<IsServiceLocatorClient C>
  std::vector<ServiceEntry> ToPythonServiceLocatorClient<C>::locate(
      const std::string& name) {
    auto release = GilRelease();
    return m_client->locate(name);
  }

  template<IsServiceLocatorClient C>
  ServiceEntry ToPythonServiceLocatorClient<C>::add(
      const std::string& name, const JsonObject& properties) {
    auto release = GilRelease();
    return m_client->add(name, properties);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::remove(const ServiceEntry& service) {
    auto release = GilRelease();
    m_client->remove(service);
  }

  template<IsServiceLocatorClient C>
  std::vector<DirectoryEntry>
      ToPythonServiceLocatorClient<C>::load_all_accounts() {
    auto release = GilRelease();
    return m_client->load_all_accounts();
  }

  template<IsServiceLocatorClient C>
  boost::optional<DirectoryEntry> ToPythonServiceLocatorClient<C>::find_account(
      const std::string& name) {
    auto release = GilRelease();
    return m_client->find_account(name);
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    auto release = GilRelease();
    return m_client->make_account(name, password, parent);
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    auto release = GilRelease();
    return m_client->make_directory(name, parent);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::store_password(
      const DirectoryEntry& account, const std::string& password) {
    auto release = GilRelease();
    m_client->store_password(account, password);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::monitor(
      ScopedQueueWriter<AccountUpdate> queue) {
    auto release = GilRelease();
    m_client->monitor(std::move(queue));
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::load_directory_entry(
      const DirectoryEntry& root, const std::string& path) {
    auto release = GilRelease();
    return m_client->load_directory_entry(root, path);
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::load_directory_entry(
      unsigned int id) {
    auto release = GilRelease();
    return m_client->load_directory_entry(id);
  }

  template<IsServiceLocatorClient C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::load_parents(
      const DirectoryEntry& entry) {
    auto release = GilRelease();
    return m_client->load_parents(entry);
  }

  template<IsServiceLocatorClient C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::load_children(
      const DirectoryEntry& entry) {
    auto release = GilRelease();
    return m_client->load_children(entry);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::remove(const DirectoryEntry& entry) {
    auto release = GilRelease();
    m_client->remove(entry);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto release = GilRelease();
    m_client->associate(entry, parent);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto release = GilRelease();
    m_client->detach(entry, parent);
  }

  template<IsServiceLocatorClient C>
  bool ToPythonServiceLocatorClient<C>::has_permissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    auto release = GilRelease();
    return m_client->has_permissions(account, target, permissions);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::store(const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    auto release = GilRelease();
    m_client->store(source, target, permissions);
  }

  template<IsServiceLocatorClient C>
  boost::posix_time::ptime
      ToPythonServiceLocatorClient<C>::load_registration_time(
        const DirectoryEntry& account) {
    auto release = GilRelease();
    return m_client->load_registration_time(account);
  }

  template<IsServiceLocatorClient C>
  boost::posix_time::ptime
      ToPythonServiceLocatorClient<C>::load_last_login_time(
        const DirectoryEntry& account) {
    auto release = GilRelease();
    return m_client->load_last_login_time(account);
  }

  template<IsServiceLocatorClient C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::rename(
      const DirectoryEntry& entry, const std::string& name) {
    auto release = GilRelease();
    return m_client->rename(entry, name);
  }

  template<IsServiceLocatorClient C>
  void ToPythonServiceLocatorClient<C>::close() {
    auto release = GilRelease();
    m_client->close();
  }
}

#endif
