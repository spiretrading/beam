#ifndef BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#include <pybind11/pybind11.h>
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"

namespace Beam::ServiceLocator {

  /**
   * Wraps a ServiceLocatorClient class for use within Python.
   * param <C> The type of ServiceLocatorClient to wrap.
   */
  template<typename C>
  class ToPythonServiceLocatorClient : public VirtualServiceLocatorClient {
    public:

      /** The type of ServiceLocatorClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonServiceLocatorClient.
       * @param client The ServiceLocatorClient to wrap.
       */
      ToPythonServiceLocatorClient(std::unique_ptr<Client> client);

      ~ToPythonServiceLocatorClient() override final;

      DirectoryEntry GetAccount() const override final;

      std::string GetSessionId() const override final;

      std::string GetEncryptedSessionId(unsigned int key) const override final;

      DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password) override final;

      DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key) override final;

      std::vector<ServiceEntry> Locate(const std::string& name) override final;

      ServiceEntry Register(const std::string& name,
        const JsonObject& properties) override final;

      std::vector<DirectoryEntry> LoadAllAccounts() override final;

      boost::optional<DirectoryEntry> FindAccount(
        const std::string& name) override final;

      DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password,
        const DirectoryEntry& parent) override final;

      DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) override final;

      void StorePassword(const DirectoryEntry& account,
        const std::string& password) override final;

      DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path) override final;

      DirectoryEntry LoadDirectoryEntry(unsigned int id) override final;

      std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override final;

      std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& entry) override final;

      void Delete(const DirectoryEntry& entry) override final;

      void Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override final;

      void Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override final;

      bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions) override final;

      void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override final;

      boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) override final;

      boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) override final;

      DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name) override final;

      void SetCredentials(const std::string& username,
        const std::string& password) override final;

      void Open() override final;

      void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonServiceLocatorClient.
   * @param client The ServiceLocatorClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonServiceLocatorClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonServiceLocatorClient<Client>>(
      std::move(client));
  }

  template<typename C>
  ToPythonServiceLocatorClient<C>::ToPythonServiceLocatorClient(
    std::unique_ptr<Client> client)
    : m_client{std::move(client)} {}

  template<typename C>
  ToPythonServiceLocatorClient<C>::~ToPythonServiceLocatorClient() {
    auto release = pybind11::gil_scoped_release();
    Close();
    m_client.reset();
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::GetAccount() const {
    auto release = pybind11::gil_scoped_release();
    return m_client->GetAccount();
  }

  template<typename C>
  std::string ToPythonServiceLocatorClient<C>::GetSessionId() const {
    auto release = pybind11::gil_scoped_release();
    return m_client->GetSessionId();
  }

  template<typename C>
  std::string ToPythonServiceLocatorClient<C>::GetEncryptedSessionId(
      unsigned int key) const {
    auto release = pybind11::gil_scoped_release();
    return m_client->GetEncryptedSessionId(key);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::AuthenticateAccount(
      const std::string& username, const std::string& password) {
    auto release = pybind11::gil_scoped_release();
    return m_client->AuthenticateAccount(username, password);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::AuthenticateSession(
      const std::string& sessionId, unsigned int key) {
    auto release = pybind11::gil_scoped_release();
    return m_client->AuthenticateSession(sessionId, key);
  }

  template<typename C>
  std::vector<ServiceEntry> ToPythonServiceLocatorClient<C>::Locate(
      const std::string& name) {
    auto release = pybind11::gil_scoped_release();
    return m_client->Locate(name);
  }

  template<typename C>
  ServiceEntry ToPythonServiceLocatorClient<C>::Register(
      const std::string& name, const JsonObject& properties) {
    auto release = pybind11::gil_scoped_release();
    return m_client->Register(name, properties);
  }

  template<typename C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::
      LoadAllAccounts() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAllAccounts();
  }

  template<typename C>
  boost::optional<DirectoryEntry> ToPythonServiceLocatorClient<C>::FindAccount(
      const std::string& name) {
    auto release = pybind11::gil_scoped_release();
    return m_client->FindAccount(name);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    auto release = pybind11::gil_scoped_release();
    return m_client->MakeAccount(name, password, parent);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    auto release = pybind11::gil_scoped_release();
    return m_client->MakeDirectory(name, parent);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    auto release = pybind11::gil_scoped_release();
    m_client->StorePassword(account, password);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::LoadDirectoryEntry(
      const DirectoryEntry& root, const std::string& path) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadDirectoryEntry(root, path);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::LoadDirectoryEntry(
      unsigned int id) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadDirectoryEntry(id);
  }

  template<typename C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::LoadParents(
      const DirectoryEntry& entry) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadParents(entry);
  }

  template<typename C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::LoadChildren(
      const DirectoryEntry& entry) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadChildren(entry);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Delete(const DirectoryEntry& entry) {
    auto release = pybind11::gil_scoped_release();
    m_client->Delete(entry);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto release = pybind11::gil_scoped_release();
    m_client->Associate(entry, parent);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto release = pybind11::gil_scoped_release();
    m_client->Detach(entry, parent);
  }

  template<typename C>
  bool ToPythonServiceLocatorClient<C>::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    auto release = pybind11::gil_scoped_release();
    return m_client->HasPermissions(account, target, permissions);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::StorePermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto release = pybind11::gil_scoped_release();
    m_client->StorePermissions(source, target, permissions);
  }

  template<typename C>
  boost::posix_time::ptime ToPythonServiceLocatorClient<C>::
      LoadRegistrationTime(const DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadRegistrationTime(account);
  }

  template<typename C>
  boost::posix_time::ptime ToPythonServiceLocatorClient<C>::LoadLastLoginTime(
      const DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadLastLoginTime(account);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    auto release = pybind11::gil_scoped_release();
    return m_client->Rename(entry, name);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::SetCredentials(
      const std::string& username, const std::string& password) {
    auto release = pybind11::gil_scoped_release();
    m_client->SetCredentials(username, password);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Open() {
    auto release = pybind11::gil_scoped_release();
    m_client->Open();
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Close() {
    m_client->Close();
  }
}

#endif
