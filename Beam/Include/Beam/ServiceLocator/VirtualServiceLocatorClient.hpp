#ifndef BEAM_VIRTUAL_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_VIRTUAL_SERVICE_LOCATOR_CLIENT_HPP
#include <memory>
#include <string>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/ServiceLocator/AccountUpdate.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Provides a pure virtual interface to a ServiceLocatorClient. */
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

      virtual void MonitorAccounts(ScopedQueueWriter<AccountUpdate> queue) = 0;

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

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualServiceLocatorClient. */
      VirtualServiceLocatorClient() = default;
  };

  /**
   * Wraps a ServiceLocatorClient providing it with a virtual interface.
   * @param C The type of ServiceLocatorClient to wrap.
   */
  template<typename C>
  class WrapperServiceLocatorClient : public VirtualServiceLocatorClient {
    public:

      /** The ServiceLocatorClient to wrap. */
      using Client = GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperServiceLocatorClient.
       * @param client The ServiceLocatorClient to wrap.
       */
      template<typename CF>
      explicit WrapperServiceLocatorClient(CF&& client);

      ~WrapperServiceLocatorClient() = default;

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
        const std::string& password, const DirectoryEntry& parent) override;

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
      GetOptionalLocalPtr<C> m_client;
  };

  /**
   * Wraps a ServiceLocatorClient into a VirtualServiceLocatorClient.
   * @param client The client to wrap.
   */
  template<typename ServiceLocatorClient>
  std::unique_ptr<VirtualServiceLocatorClient> MakeVirtualServiceLocatorClient(
      ServiceLocatorClient&& client) {
    return std::make_unique<WrapperServiceLocatorClient<ServiceLocatorClient>>(
      std::forward<ServiceLocatorClient>(client));
  }

  template<typename C>
  template<typename CF>
  WrapperServiceLocatorClient<C>::WrapperServiceLocatorClient(CF&& client)
      : m_client(std::forward<CF>(client)) {}

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::GetAccount() const {
    return m_client->GetAccount();
  }

  template<typename C>
  std::string WrapperServiceLocatorClient<C>::GetSessionId() const {
    return m_client->GetSessionId();
  }

  template<typename C>
  std::string WrapperServiceLocatorClient<C>::GetEncryptedSessionId(
      unsigned int key) const {
    return m_client->GetEncryptedSessionId(key);
  }

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::AuthenticateAccount(
      const std::string& username, const std::string& password) {
    return m_client->AuthenticateAccount(username, password);
  }

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::AuthenticateSession(
      const std::string& sessionId, unsigned int key) {
    return m_client->AuthenticateSession(sessionId, key);
  }

  template<typename C>
  std::vector<ServiceEntry> WrapperServiceLocatorClient<C>::Locate(
      const std::string& name) {
    return m_client->Locate(name);
  }

  template<typename C>
  ServiceEntry WrapperServiceLocatorClient<C>::Register(const std::string& name,
      const JsonObject& properties) {
    return m_client->Register(name, properties);
  }

  template<typename C>
  std::vector<DirectoryEntry> WrapperServiceLocatorClient<C>::
      LoadAllAccounts() {
    return m_client->LoadAllAccounts();
  }

  template<typename C>
  boost::optional<DirectoryEntry> WrapperServiceLocatorClient<C>::FindAccount(
      const std::string& name) {
    return m_client->FindAccount(name);
  }

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    return m_client->MakeAccount(name, password, parent);
  }

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    return m_client->MakeDirectory(name, parent);
  }

  template<typename C>
  void WrapperServiceLocatorClient<C>::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    m_client->StorePassword(account, password);
  }

  template<typename C>
  void WrapperServiceLocatorClient<C>::MonitorAccounts(
      ScopedQueueWriter<AccountUpdate> queue) {
    return m_client->MonitorAccounts(std::move(queue));
  }

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::LoadDirectoryEntry(
      const DirectoryEntry& root, const std::string& path) {
    return m_client->LoadDirectoryEntry(root, path);
  }

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::LoadDirectoryEntry(
      unsigned int id) {
    return m_client->LoadDirectoryEntry(id);
  }

  template<typename C>
  std::vector<DirectoryEntry> WrapperServiceLocatorClient<C>::LoadParents(
      const DirectoryEntry& entry) {
    return m_client->LoadParents(entry);
  }

  template<typename C>
  std::vector<DirectoryEntry> WrapperServiceLocatorClient<C>::LoadChildren(
      const DirectoryEntry& entry) {
    return m_client->LoadChildren(entry);
  }

  template<typename C>
  void WrapperServiceLocatorClient<C>::Delete(const DirectoryEntry& entry) {
    m_client->Delete(entry);
  }

  template<typename C>
  void WrapperServiceLocatorClient<C>::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    m_client->Associate(entry, parent);
  }

  template<typename C>
  void WrapperServiceLocatorClient<C>::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    m_client->Detach(entry, parent);
  }

  template<typename C>
  bool WrapperServiceLocatorClient<C>::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return m_client->HasPermissions(account, target, permissions);
  }

  template<typename C>
  void WrapperServiceLocatorClient<C>::StorePermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_client->StorePermissions(source, target, permissions);
  }

  template<typename C>
  boost::posix_time::ptime WrapperServiceLocatorClient<C>::LoadRegistrationTime(
      const DirectoryEntry& account) {
    return m_client->LoadRegistrationTime(account);
  }

  template<typename C>
  boost::posix_time::ptime WrapperServiceLocatorClient<C>::LoadLastLoginTime(
      const DirectoryEntry& account) {
    return m_client->LoadLastLoginTime(account);
  }

  template<typename C>
  DirectoryEntry WrapperServiceLocatorClient<C>::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    return m_client->Rename(entry, name);
  }

  template<typename C>
  void WrapperServiceLocatorClient<C>::Close() {
    m_client->Close();
  }
}

#endif
