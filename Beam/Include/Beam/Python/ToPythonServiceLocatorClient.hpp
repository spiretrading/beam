#ifndef BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"

namespace Beam::ServiceLocator {

  /**
   * Wraps a ServiceLocatorClient class for use within Python.
   * param <C> The type of ServiceLocatorClient to wrap.
   */
  template<typename C>
  class ToPythonServiceLocatorClient final :
      public VirtualServiceLocatorClient {
    public:

      /** The type of ServiceLocatorClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonServiceLocatorClient.
       * @param client The ServiceLocatorClient to wrap.
       */
      explicit ToPythonServiceLocatorClient(std::unique_ptr<Client> client);

      ~ToPythonServiceLocatorClient() override;

      DirectoryEntry GetAccount() const override;

      std::string GetSessionId() const override;

      std::string GetEncryptedSessionId(unsigned int key) const override;

      DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password) override;

      DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key) override;

      std::vector<ServiceEntry> Locate(const std::string& name) override;

      ServiceEntry Register(const std::string& name,
        const JsonObject& properties) override;

      std::vector<DirectoryEntry> LoadAllAccounts() override;

      boost::optional<DirectoryEntry> FindAccount(
        const std::string& name) override;

      DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password,
        const DirectoryEntry& parent) override;

      DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) override;

      void StorePassword(const DirectoryEntry& account,
        const std::string& password) override;

      void MonitorAccounts(ScopedQueueWriter<AccountUpdate> queue) override;

      DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path) override;

      DirectoryEntry LoadDirectoryEntry(unsigned int id) override;

      std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override;

      std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& entry) override;

      void Delete(const DirectoryEntry& entry) override;

      void Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      void Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions) override;

      void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override;

      boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) override;

      boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) override;

      DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name) override;

      void Close() override;

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
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonServiceLocatorClient<C>::~ToPythonServiceLocatorClient() {
    Close();
    auto release = Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::GetAccount() const {
    auto release = Python::GilRelease();
    return m_client->GetAccount();
  }

  template<typename C>
  std::string ToPythonServiceLocatorClient<C>::GetSessionId() const {
    auto release = Python::GilRelease();
    return m_client->GetSessionId();
  }

  template<typename C>
  std::string ToPythonServiceLocatorClient<C>::GetEncryptedSessionId(
      unsigned int key) const {
    auto release = Python::GilRelease();
    return m_client->GetEncryptedSessionId(key);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::AuthenticateAccount(
      const std::string& username, const std::string& password) {
    auto release = Python::GilRelease();
    return m_client->AuthenticateAccount(username, password);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::AuthenticateSession(
      const std::string& sessionId, unsigned int key) {
    auto release = Python::GilRelease();
    return m_client->AuthenticateSession(sessionId, key);
  }

  template<typename C>
  std::vector<ServiceEntry> ToPythonServiceLocatorClient<C>::Locate(
      const std::string& name) {
    auto release = Python::GilRelease();
    return m_client->Locate(name);
  }

  template<typename C>
  ServiceEntry ToPythonServiceLocatorClient<C>::Register(
      const std::string& name, const JsonObject& properties) {
    auto release = Python::GilRelease();
    return m_client->Register(name, properties);
  }

  template<typename C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::
      LoadAllAccounts() {
    auto release = Python::GilRelease();
    return m_client->LoadAllAccounts();
  }

  template<typename C>
  boost::optional<DirectoryEntry> ToPythonServiceLocatorClient<C>::FindAccount(
      const std::string& name) {
    auto release = Python::GilRelease();
    return m_client->FindAccount(name);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    auto release = Python::GilRelease();
    return m_client->MakeAccount(name, password, parent);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    auto release = Python::GilRelease();
    return m_client->MakeDirectory(name, parent);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    auto release = Python::GilRelease();
    m_client->StorePassword(account, password);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::MonitorAccounts(
      ScopedQueueWriter<AccountUpdate> queue) {
    auto release = Python::GilRelease();
    m_client->MonitorAccounts(std::move(queue));
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::LoadDirectoryEntry(
      const DirectoryEntry& root, const std::string& path) {
    auto release = Python::GilRelease();
    return m_client->LoadDirectoryEntry(root, path);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::LoadDirectoryEntry(
      unsigned int id) {
    auto release = Python::GilRelease();
    return m_client->LoadDirectoryEntry(id);
  }

  template<typename C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::LoadParents(
      const DirectoryEntry& entry) {
    auto release = Python::GilRelease();
    return m_client->LoadParents(entry);
  }

  template<typename C>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<C>::LoadChildren(
      const DirectoryEntry& entry) {
    auto release = Python::GilRelease();
    return m_client->LoadChildren(entry);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Delete(const DirectoryEntry& entry) {
    auto release = Python::GilRelease();
    m_client->Delete(entry);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto release = Python::GilRelease();
    m_client->Associate(entry, parent);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto release = Python::GilRelease();
    m_client->Detach(entry, parent);
  }

  template<typename C>
  bool ToPythonServiceLocatorClient<C>::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    auto release = Python::GilRelease();
    return m_client->HasPermissions(account, target, permissions);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::StorePermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto release = Python::GilRelease();
    m_client->StorePermissions(source, target, permissions);
  }

  template<typename C>
  boost::posix_time::ptime ToPythonServiceLocatorClient<C>::
      LoadRegistrationTime(const DirectoryEntry& account) {
    auto release = Python::GilRelease();
    return m_client->LoadRegistrationTime(account);
  }

  template<typename C>
  boost::posix_time::ptime ToPythonServiceLocatorClient<C>::LoadLastLoginTime(
      const DirectoryEntry& account) {
    auto release = Python::GilRelease();
    return m_client->LoadLastLoginTime(account);
  }

  template<typename C>
  DirectoryEntry ToPythonServiceLocatorClient<C>::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    auto release = Python::GilRelease();
    return m_client->Rename(entry, name);
  }

  template<typename C>
  void ToPythonServiceLocatorClient<C>::Close() {
    auto release = Python::GilRelease();
    m_client->Close();
  }
}

#endif
