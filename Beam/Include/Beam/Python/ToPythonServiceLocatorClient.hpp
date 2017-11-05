#ifndef BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_TO_PYTHON_SERVICE_LOCATOR_CLIENT_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class ToPythonServiceLocatorClient
      \brief Wraps a ServiceLocatorClient class for use within Python.
      \tparam ClientType The type of ServiceLocatorClient to wrap.
   */
  template<typename ClientType>
  class ToPythonServiceLocatorClient : public VirtualServiceLocatorClient {
    public:

      //! The type of ServiceLocatorClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonServiceLocatorClient.
      /*!
        \param client The ServiceLocatorClient to wrap.
      */
      ToPythonServiceLocatorClient(std::unique_ptr<Client> client);

      virtual ~ToPythonServiceLocatorClient() override final;

      virtual DirectoryEntry GetAccount() const override final;

      virtual std::string GetSessionId() const override final;

      virtual std::string GetEncryptedSessionId(
        unsigned int key) const override final;

      virtual DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password) override final;

      virtual DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key) override final;

      virtual std::vector<ServiceEntry> Locate(
        const std::string& name) override final;

      virtual ServiceEntry Register(const std::string& name,
        const JsonObject& properties) override final;

      virtual std::vector<DirectoryEntry> LoadAllAccounts() override final;

      virtual boost::optional<DirectoryEntry> FindAccount(
        const std::string& name) override final;

      virtual DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password,
        const DirectoryEntry& parent) override final;

      virtual DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) override final;

      virtual void StorePassword(const DirectoryEntry& account,
        const std::string& password) override final;

      virtual DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path) override final;

      virtual DirectoryEntry LoadDirectoryEntry(unsigned int id) override final;

      virtual std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override final;

      virtual std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& entry) override final;

      virtual void Delete(const DirectoryEntry& entry) override final;

      virtual void Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override final;

      virtual void Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override final;

      virtual bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions) override final;

      virtual void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override final;

      virtual boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) override final;

      virtual boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) override final;

      virtual DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name) override final;

      virtual void SetCredentials(const std::string& username,
        const std::string& password) override final;

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<ClientType> m_client;
      IO::OpenState m_openState;

      void Shutdown();
  };

  //! Makes a ToPythonServiceLocatorClient.
  /*!
    \param client The ServiceLocatorClient to wrap.
  */
  template<typename Client>
  std::unique_ptr<ToPythonServiceLocatorClient<Client>>
      MakeToPythonServiceLocatorClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonServiceLocatorClient<Client>>(
      std::move(client));
  }

  template<typename ClientType>
  ToPythonServiceLocatorClient<ClientType>::ToPythonServiceLocatorClient(
      std::unique_ptr<ClientType> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonServiceLocatorClient<ClientType>::~ToPythonServiceLocatorClient() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    Close();
    m_client.reset();
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::GetAccount() const {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->GetAccount();
  }

  template<typename ClientType>
  std::string ToPythonServiceLocatorClient<ClientType>::GetSessionId() const {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->GetSessionId();
  }

  template<typename ClientType>
  std::string ToPythonServiceLocatorClient<ClientType>::GetEncryptedSessionId(
      unsigned int key) const {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->GetEncryptedSessionId(key);
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::AuthenticateAccount(
      const std::string& username, const std::string& password) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->AuthenticateAccount(username, password);
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::AuthenticateSession(
      const std::string& sessionId, unsigned int key) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->AuthenticateSession(sessionId, key);
  }

  template<typename ClientType>
  std::vector<ServiceEntry> ToPythonServiceLocatorClient<ClientType>::Locate(
      const std::string& name) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->Locate(name);
  }

  template<typename ClientType>
  ServiceEntry ToPythonServiceLocatorClient<ClientType>::Register(
      const std::string& name, const JsonObject& properties) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->Register(name, properties);
  }

  template<typename ClientType>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<ClientType>::
      LoadAllAccounts() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadAllAccounts();
  }

  template<typename ClientType>
  boost::optional<DirectoryEntry> ToPythonServiceLocatorClient<ClientType>::
      FindAccount(const std::string& name) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->FindAccount(name);
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->MakeAccount(name, password, parent);
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->MakeDirectory(name, parent);
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->StorePassword(account, password);
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::LoadDirectoryEntry(
      const DirectoryEntry& root, const std::string& path) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadDirectoryEntry(root, path);
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::LoadDirectoryEntry(
      unsigned int id) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadDirectoryEntry(id);
  }

  template<typename ClientType>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<ClientType>::
      LoadParents(const DirectoryEntry& entry) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadParents(entry);
  }

  template<typename ClientType>
  std::vector<DirectoryEntry> ToPythonServiceLocatorClient<ClientType>::
      LoadChildren(const DirectoryEntry& entry) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadChildren(entry);
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::Delete(
      const DirectoryEntry& entry) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->Delete(entry);
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->Associate(entry, parent);
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::Detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->Detach(entry, parent);
  }

  template<typename ClientType>
  bool ToPythonServiceLocatorClient<ClientType>::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->HasPermissions(account, target, permissions);
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::StorePermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->StorePermissions(source, target, permissions);
  }

  template<typename ClientType>
  boost::posix_time::ptime ToPythonServiceLocatorClient<ClientType>::
      LoadRegistrationTime(const DirectoryEntry& account) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadRegistrationTime(account);
  }

  template<typename ClientType>
  boost::posix_time::ptime ToPythonServiceLocatorClient<ClientType>::
      LoadLastLoginTime(const DirectoryEntry& account) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadLastLoginTime(account);
  }

  template<typename ClientType>
  DirectoryEntry ToPythonServiceLocatorClient<ClientType>::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->Rename(entry, name);
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::SetCredentials(
      const std::string& username, const std::string& password) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->SetCredentials(username, password);
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::Open() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::Close() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonServiceLocatorClient<ClientType>::Shutdown() {
    m_client->Close();
    m_openState.SetClosed();
  }
}
}

#endif
