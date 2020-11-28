#ifndef BEAM_SERVICE_LOCATOR_CLIENT_BOX_HPP
#define BEAM_SERVICE_LOCATOR_CLIENT_BOX_HPP
#include <memory>
#include <random>
#include <string>
#include <vector>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/ServiceLocator/AccountUpdate.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Provides a generic interface over an arbitrary ServiceLocatorClient. */
  class ServiceLocatorClientBox {
    public:

      /**
       * Constructs a ServiceLocatorClientBox of a specified type using
       * emplacement.
       * @param <T> The type of client to emplace.
       * @param args The arguments to pass to the emplaced client.
       */
      template<typename T, typename... Args>
      explicit ServiceLocatorClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ServiceLocatorClientBox by copying an existing client.
       * @param client The client to copy.
       */
      template<typename ServiceLocatorClient>
      explicit ServiceLocatorClientBox(ServiceLocatorClient client);

      explicit ServiceLocatorClientBox(ServiceLocatorClientBox* client);

      explicit ServiceLocatorClientBox(
        const std::shared_ptr<ServiceLocatorClientBox>& client);

      explicit ServiceLocatorClientBox(
        const std::unique_ptr<ServiceLocatorClientBox>& client);

      DirectoryEntry GetAccount() const;

      std::string GetSessionId() const;

      std::string GetEncryptedSessionId(unsigned int key) const;

      DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password);

      DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key);

      std::vector<ServiceEntry> Locate(const std::string& name);

      ServiceEntry Register(const std::string& name,
        const JsonObject& properties);

      void Unregister(const ServiceEntry& service);

      std::vector<DirectoryEntry> LoadAllAccounts();

      boost::optional<DirectoryEntry> FindAccount(const std::string& name);

      DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent);

      DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent);

      void StorePassword(const DirectoryEntry& account,
        const std::string& password);

      void MonitorAccounts(ScopedQueueWriter<AccountUpdate> queue);

      DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path);

      DirectoryEntry LoadDirectoryEntry(unsigned int id);

      std::vector<DirectoryEntry> LoadParents(const DirectoryEntry& entry);

      std::vector<DirectoryEntry> LoadChildren(const DirectoryEntry& entry);

      void Delete(const DirectoryEntry& entry);

      void Associate(const DirectoryEntry& entry, const DirectoryEntry& parent);

      void Detach(const DirectoryEntry& entry, const DirectoryEntry& parent);

      bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions);

      void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions);

      boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account);

      boost::posix_time::ptime LoadLastLoginTime(const DirectoryEntry& account);

      DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name);

      void Close();

    private:
      struct VirtualServiceLocatorClient {
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
        virtual void Unregister(const ServiceEntry& service) = 0;
        virtual std::vector<DirectoryEntry> LoadAllAccounts() = 0;
        virtual boost::optional<DirectoryEntry> FindAccount(
          const std::string& name) = 0;
        virtual DirectoryEntry MakeAccount(const std::string& name,
          const std::string& password, const DirectoryEntry& parent) = 0;
        virtual DirectoryEntry MakeDirectory(const std::string& name,
          const DirectoryEntry& parent) = 0;
        virtual void StorePassword(const DirectoryEntry& account,
          const std::string& password) = 0;
        virtual void MonitorAccounts(
          ScopedQueueWriter<AccountUpdate> queue) = 0;
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
      };
      template<typename C>
      struct WrappedServiceLocatorClient final : VirtualServiceLocatorClient {
        using Client = C;
        GetOptionalLocalPtr<Client> m_client;

        template<typename... Args>
        WrappedServiceLocatorClient(Args&&... args);
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
        void Unregister(const ServiceEntry& service) override;
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
      };
      std::shared_ptr<VirtualServiceLocatorClient> m_client;
  };

  /**
   * Loads a directory, or creates it if it doesn't already exist.
   * @param serviceLocatorClient The ServiceLocatorClient to use.
   * @param name The name of the directory to load or create.
   * @param parent The directory's parent.
   * @return directory The directory that was loaded.
   */
  template<typename ServiceLocatorClient>
  DirectoryEntry LoadOrCreateDirectory(ServiceLocatorClient& client,
      const std::string& name, const DirectoryEntry& parent) {
    try {
      return client.LoadDirectoryEntry(parent, name);
    } catch(const Services::ServiceRequestException&) {
      return client.MakeDirectory(name, parent);
    }
  }

  /**
   * Locates the IP addresses of a service.
   * @param client The ServiceLocatorClient used to locate the addresses.
   * @param serviceName The name of the service to locate.
   * @param servicePredicate A function to apply to a ServiceEntry to determine
   *        if it matches some criteria.
   * @return The list of IP addresses for the specified service.
   */
  template<typename ServiceLocatorClient, typename ServicePredicate>
  std::vector<Network::IpAddress> LocateServiceAddresses(
      ServiceLocatorClient& client, const std::string& serviceName,
      ServicePredicate servicePredicate) {
    auto services = std::vector<ServiceEntry>();
    try {
      services = client.Locate(serviceName);
    } catch(const std::exception&) {
      BOOST_THROW_EXCEPTION(IO::ConnectException(
        "No " + serviceName + " services available."));
    }
    services.erase(std::remove_if(services.begin(), services.end(),
      [&] (auto& entry) {
        return !servicePredicate(entry);
      }), services.end());
    if(services.empty()) {
      BOOST_THROW_EXCEPTION(IO::ConnectException(
        "No " + serviceName + " services available."));
    }
    auto seed = std::random_device();
    auto generator = std::mt19937(seed());
    auto distribution = std::uniform_int_distribution<std::size_t>(
      0, services.size() - 1);
    auto& service = services[distribution(generator)];
    auto addresses = Parsers::Parse<std::vector<Network::IpAddress>>(
      boost::get<std::string>(service.GetProperties().At("addresses")));
    return addresses;
  }

  /**
   * Locates the IP addresses of a service.
   * @param client The ServiceLocatorClient used to locate the addresses.
   * @param serviceName The name of the service to locate.
   * @return The list of IP addresses for the specified service.
   */
  template<typename ServiceLocatorClient>
  std::vector<Network::IpAddress> LocateServiceAddresses(
      ServiceLocatorClient& client, const std::string& serviceName) {
    return LocateServiceAddresses(client, serviceName,
      [] (auto&&) {
        return true;
      });
  }

  template<typename T, typename... Args>
  ServiceLocatorClientBox::ServiceLocatorClientBox(std::in_place_type_t<T>,
    Args&&... args)
    : m_client(std::make_shared<WrappedServiceLocatorClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename ServiceLocatorClient>
  ServiceLocatorClientBox::ServiceLocatorClientBox(ServiceLocatorClient client)
    : ServiceLocatorClientBox(std::in_place_type<ServiceLocatorClientBox>,
        std::move(client)) {}

  inline ServiceLocatorClientBox::ServiceLocatorClientBox(
    ServiceLocatorClientBox* client)
    : ServiceLocatorClientBox(*client) {}

  inline ServiceLocatorClientBox::ServiceLocatorClientBox(
    const std::shared_ptr<ServiceLocatorClientBox>& client)
    : ServiceLocatorClientBox(*client) {}

  inline ServiceLocatorClientBox::ServiceLocatorClientBox(
    const std::unique_ptr<ServiceLocatorClientBox>& client)
    : ServiceLocatorClientBox(*client) {}

  inline DirectoryEntry ServiceLocatorClientBox::GetAccount() const {
    return m_client->GetAccount();
  }

  inline std::string ServiceLocatorClientBox::GetSessionId() const {
    return m_client->GetSessionId();
  }

  inline std::string ServiceLocatorClientBox::GetEncryptedSessionId(
      unsigned int key) const {
    return m_client->GetEncryptedSessionId(key);
  }

  inline DirectoryEntry ServiceLocatorClientBox::AuthenticateAccount(
      const std::string& username, const std::string& password) {
    return m_client->AuthenticateAccount(username, password);
  }

  inline DirectoryEntry ServiceLocatorClientBox::AuthenticateSession(
      const std::string& sessionId, unsigned int key) {
    return m_client->AuthenticateSession(sessionId, key);
  }

  inline std::vector<ServiceEntry> ServiceLocatorClientBox::Locate(
      const std::string& name) {
    return m_client->Locate(name);
  }

  inline ServiceEntry ServiceLocatorClientBox::Register(const std::string& name,
      const JsonObject& properties) {
    return m_client->Register(name, properties);
  }

  inline void ServiceLocatorClientBox::Unregister(const ServiceEntry& service) {
    m_client->Unregister(service);
  }

  inline std::vector<DirectoryEntry>
      ServiceLocatorClientBox::LoadAllAccounts() {
    return m_client->LoadAllAccounts();
  }

  inline boost::optional<DirectoryEntry> ServiceLocatorClientBox::FindAccount(
      const std::string& name) {
    return m_client->FindAccount(name);
  }

  inline DirectoryEntry ServiceLocatorClientBox::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    return m_client->MakeAccount(name, password, parent);
  }

  inline DirectoryEntry ServiceLocatorClientBox::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    return m_client->MakeDirectory(name, parent);
  }

  inline void ServiceLocatorClientBox::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    m_client->StorePassword(account, password);
  }

  inline void ServiceLocatorClientBox::MonitorAccounts(
      ScopedQueueWriter<AccountUpdate> queue) {
    m_client->MonitorAccounts(std::move(queue));
  }

  inline DirectoryEntry ServiceLocatorClientBox::LoadDirectoryEntry(
      const DirectoryEntry& root, const std::string& path) {
    return m_client->LoadDirectoryEntry(root, path);
  }

  inline DirectoryEntry ServiceLocatorClientBox::LoadDirectoryEntry(
      unsigned int id) {
    return m_client->LoadDirectoryEntry(id);
  }

  inline std::vector<DirectoryEntry> ServiceLocatorClientBox::LoadParents(
      const DirectoryEntry& entry) {
    return m_client->LoadParents(entry);
  }

  inline std::vector<DirectoryEntry> ServiceLocatorClientBox::LoadChildren(
      const DirectoryEntry& entry) {
    return m_client->LoadChildren(entry);
  }

  inline void ServiceLocatorClientBox::Delete(const DirectoryEntry& entry) {
    m_client->Delete(entry);
  }

  inline void ServiceLocatorClientBox::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    m_client->Associate(entry, parent);
  }

  inline void ServiceLocatorClientBox::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    m_client->Detach(entry, parent);
  }

  inline bool ServiceLocatorClientBox::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return m_client->HasPermissions(account, target, permissions);
  }

  inline void ServiceLocatorClientBox::StorePermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_client->StorePermissions(source, target, permissions);
  }

  inline boost::posix_time::ptime ServiceLocatorClientBox::LoadRegistrationTime(
      const DirectoryEntry& account) {
    return m_client->LoadRegistrationTime(account);
  }

  inline boost::posix_time::ptime ServiceLocatorClientBox::LoadLastLoginTime(
      const DirectoryEntry& account) {
    return m_client->LoadLastLoginTime(account);
  }

  inline DirectoryEntry ServiceLocatorClientBox::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    return m_client->Rename(entry, name);
  }

  inline void ServiceLocatorClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  ServiceLocatorClientBox::WrappedServiceLocatorClient<
    C>::WrappedServiceLocatorClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::GetAccount() const {
    return m_client->GetAccount();
  }

  template<typename C>
  std::string ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::GetSessionId() const {
    return m_client->GetSessionId();
  }

  template<typename C>
  std::string ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::GetEncryptedSessionId(unsigned int key) const {
    return m_client->GetEncryptedSessionId(key);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::AuthenticateAccount(const std::string& username,
      const std::string& password) {
    return m_client->AuthenticateAccount(username, password);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::AuthenticateSession(const std::string& sessionId, unsigned int key) {
    return m_client->AuthenticateSession(sessionId, key);
  }

  template<typename C>
  std::vector<ServiceEntry>
      ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::Locate(
      const std::string& name) {
    return m_client->Locate(name);
  }

  template<typename C>
  ServiceEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::Register(const std::string& name, const JsonObject& properties) {
    return m_client->Register(name, properties);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::Unregister(
      const ServiceEntry& service) {
    m_client->Unregister(service);
  }

  template<typename C>
  std::vector<DirectoryEntry>
      ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::LoadAllAccounts() {
    return m_client->LoadAllAccounts();
  }

  template<typename C>
  boost::optional<DirectoryEntry>
      ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::FindAccount(
      const std::string& name) {
    return m_client->FindAccount(name);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::MakeAccount(const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    return m_client->MakeAccount(name, password, parent);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::MakeDirectory(const std::string& name, const DirectoryEntry& parent) {
    return m_client->MakeDirectory(name, parent);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    m_client->StorePassword(account, password);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::MonitorAccounts(
      ScopedQueueWriter<AccountUpdate> queue) {
    m_client->MonitorAccounts(std::move(queue));
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::LoadDirectoryEntry(const DirectoryEntry& root,
      const std::string& path) {
    return m_client->LoadDirectoryEntry(root, path);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::LoadDirectoryEntry(unsigned int id) {
    return m_client->LoadDirectoryEntry(id);
  }

  template<typename C>
  std::vector<DirectoryEntry>
      ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::LoadParents(
      const DirectoryEntry& entry) {
    return m_client->LoadParents(entry);
  }

  template<typename C>
  std::vector<DirectoryEntry>
      ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::LoadChildren(
      const DirectoryEntry& entry) {
    return m_client->LoadChildren(entry);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::Delete(
      const DirectoryEntry& entry) {
    m_client->Delete(entry);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->Associate(entry, parent);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::Detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->Detach(entry, parent);
  }

  template<typename C>
  bool ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return m_client->HasPermissions(account, target, permissions);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::StorePermissions(const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    m_client->StorePermissions(source, target, permissions);
  }

  template<typename C>
  boost::posix_time::ptime ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::LoadRegistrationTime(const DirectoryEntry& account) {
    return m_client->LoadRegistrationTime(account);
  }

  template<typename C>
  boost::posix_time::ptime ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::LoadLastLoginTime(const DirectoryEntry& account) {
    return m_client->LoadLastLoginTime(account);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClientBox::WrappedServiceLocatorClient<
      C>::Rename(const DirectoryEntry& entry, const std::string& name) {
    return m_client->Rename(entry, name);
  }

  template<typename C>
  void ServiceLocatorClientBox::WrappedServiceLocatorClient<C>::Close() {
    m_client->Close();
  }
}

#endif
