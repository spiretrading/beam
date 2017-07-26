#ifndef BEAM_VIRTUALSERVICELOCATORCLIENT_HPP
#define BEAM_VIRTUALSERVICELOCATORCLIENT_HPP
#include <memory>
#include <string>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/UniquePtr.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Json/JsonObject.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class VirtualServiceLocatorClient
      \brief Provides a pure virtual interface to a ServiceLocatorClient.
   */
  class VirtualServiceLocatorClient : private boost::noncopyable {
    public:
      virtual ~VirtualServiceLocatorClient() = default;

      virtual DirectoryEntry GetAccount() const = 0;

      virtual std::string GetSessionId() const = 0;

      virtual std::string GetEncryptedSessionId(unsigned int key) const = 0;

      virtual DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password) = 0;

      virtual DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key) = 0;

      virtual std::vector<ServiceEntry> Locate(const std::string& name) = 0;

      virtual ServiceEntry Register(const std::string& name,
        const JsonObject& properties) = 0;

      virtual std::vector<DirectoryEntry> LoadAllAccounts() = 0;

      virtual boost::optional<DirectoryEntry> FindAccount(
        const std::string& name) = 0;

      virtual DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent) = 0;

      virtual DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) = 0;

      virtual void StorePassword(const DirectoryEntry& account,
        const std::string& password) = 0;

      virtual DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path) = 0;

      virtual DirectoryEntry LoadDirectoryEntry(unsigned int id) = 0;

      virtual std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) = 0;

      virtual std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& entry) = 0;

      virtual void Delete(const DirectoryEntry& entry) = 0;

      virtual void Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) = 0;

      virtual void Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) = 0;

      virtual bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions) = 0;

      virtual void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) = 0;

      virtual boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) = 0;

      virtual boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) = 0;

      virtual DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name) = 0;

      virtual void SetCredentials(const std::string& username,
        const std::string& password) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualServiceLocatorClient.
      VirtualServiceLocatorClient() = default;
  };

  /*! \class WrapperServiceLocatorClient
      \brief Wraps a ServiceLocatorClient providing it with a virtual interface.
      \tparam ClientType The type of ServiceLocatorClient to wrap.
   */
  template<typename ClientType>
  class WrapperServiceLocatorClient : public VirtualServiceLocatorClient {
    public:

      //! The ServiceLocatorClient to wrap.
      using Client = GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperServiceLocatorClient.
      /*!
        \param client The ServiceLocatorClient to wrap.
      */
      template<typename ServiceLocatorClientForward>
      WrapperServiceLocatorClient(ServiceLocatorClientForward&& client);

      virtual ~WrapperServiceLocatorClient() = default;

      virtual DirectoryEntry GetAccount() const;

      virtual std::string GetSessionId() const;

      virtual std::string GetEncryptedSessionId(unsigned int key) const;

      virtual DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password);

      virtual DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key);

      virtual std::vector<ServiceEntry> Locate(const std::string& name);

      virtual ServiceEntry Register(const std::string& name,
        const JsonObject& properties);

      virtual std::vector<DirectoryEntry> LoadAllAccounts();

      virtual boost::optional<DirectoryEntry> FindAccount(
        const std::string& name);

      virtual DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent);

      virtual DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent);

      virtual void StorePassword(const DirectoryEntry& account,
        const std::string& password);

      virtual DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path);

      virtual DirectoryEntry LoadDirectoryEntry(unsigned int id);

      virtual std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry);

      virtual std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& entry);

      virtual void Delete(const DirectoryEntry& entry);

      virtual void Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent);

      virtual void Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent);

      virtual bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions);

      virtual void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions);

      virtual boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account);

      virtual boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account);

      virtual DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name);

      virtual void SetCredentials(const std::string& username,
        const std::string& password);

      virtual void Open();

      virtual void Close();

    private:
      GetOptionalLocalPtr<ClientType> m_client;
  };

  //! Wraps a ServiceLocatorClient into a VirtualServiceLocatorClient.
  /*!
    \param client The client to wrap.
  */
  template<typename ServiceLocatorClient>
  std::unique_ptr<VirtualServiceLocatorClient> MakeVirtualServiceLocatorClient(
      ServiceLocatorClient&& client) {
    return std::make_unique<WrapperServiceLocatorClient<ServiceLocatorClient>>(
      std::forward<ServiceLocatorClient>(client));
  }

  template<typename ClientType>
  template<typename ServiceLocatorClientForward>
  WrapperServiceLocatorClient<ClientType>::WrapperServiceLocatorClient(
      ServiceLocatorClientForward&& client)
      : m_client{std::forward<ServiceLocatorClientForward>(client)} {}

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::GetAccount() const {
    return m_client->GetAccount();
  }

  template<typename ClientType>
  std::string WrapperServiceLocatorClient<ClientType>::GetSessionId() const {
    return m_client->GetSessionId();
  }

  template<typename ClientType>
  std::string WrapperServiceLocatorClient<ClientType>::GetEncryptedSessionId(
      unsigned int key) const {
    return m_client->GetEncryptedSessionId(key);
  }

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::AuthenticateAccount(
      const std::string& username, const std::string& password) {
    return m_client->AuthenticateAccount(username, password);
  }

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::AuthenticateSession(
      const std::string& sessionId, unsigned int key) {
    return m_client->AuthenticateSession(sessionId, key);
  }

  template<typename ClientType>
  std::vector<ServiceEntry> WrapperServiceLocatorClient<ClientType>::Locate(
      const std::string& name) {
    return m_client->Locate(name);
  }

  template<typename ClientType>
  ServiceEntry WrapperServiceLocatorClient<ClientType>::Register(
      const std::string& name, const JsonObject& properties) {
    return m_client->Register(name, properties);
  }

  template<typename ClientType>
  std::vector<DirectoryEntry> WrapperServiceLocatorClient<ClientType>::
      LoadAllAccounts() {
    return m_client->LoadAllAccounts();
  }

  template<typename ClientType>
  boost::optional<DirectoryEntry> WrapperServiceLocatorClient<ClientType>::
      FindAccount(const std::string& name) {
    return m_client->FindAccount(name);
  }

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    return m_client->MakeAccount(name, password, parent);
  }

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    return m_client->MakeDirectory(name, parent);
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    m_client->StorePassword(account, password);
  }

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::LoadDirectoryEntry(
      const DirectoryEntry& root, const std::string& path) {
    return m_client->LoadDirectoryEntry(root, path);
  }

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::LoadDirectoryEntry(
      unsigned int id) {
    return m_client->LoadDirectoryEntry(id);
  }

  template<typename ClientType>
  std::vector<DirectoryEntry> WrapperServiceLocatorClient<ClientType>::
      LoadParents(const DirectoryEntry& entry) {
    return m_client->LoadParents(entry);
  }

  template<typename ClientType>
  std::vector<DirectoryEntry> WrapperServiceLocatorClient<ClientType>::
      LoadChildren(const DirectoryEntry& entry) {
    return m_client->LoadChildren(entry);
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::Delete(
      const DirectoryEntry& entry) {
    m_client->Delete(entry);
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->Associate(entry, parent);
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::Detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->Detach(entry, parent);
  }

  template<typename ClientType>
  bool WrapperServiceLocatorClient<ClientType>::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return m_client->HasPermissions(account, target, permissions);
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::StorePermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_client->StorePermissions(source, target, permissions);
  }

  template<typename ClientType>
  boost::posix_time::ptime WrapperServiceLocatorClient<ClientType>::
      LoadRegistrationTime(const DirectoryEntry& account) {
    return m_client->LoadRegistrationTime(account);
  }

  template<typename ClientType>
  boost::posix_time::ptime WrapperServiceLocatorClient<ClientType>::
      LoadLastLoginTime(const DirectoryEntry& account) {
    return m_client->LoadLastLoginTime(account);
  }

  template<typename ClientType>
  DirectoryEntry WrapperServiceLocatorClient<ClientType>::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    return m_client->Rename(entry, name);
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::SetCredentials(
      const std::string& username, const std::string& password) {
    m_client->SetCredentials(username, password);
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperServiceLocatorClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
