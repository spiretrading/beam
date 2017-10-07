#ifndef BEAM_SERVICELOCATORCLIENT_HPP
#define BEAM_SERVICELOCATORCLIENT_HPP
#include <random>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class ServiceLocatorClient
      \brief Client used to locate services on a service network.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class ServiceLocatorClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder = GetTryDereferenceType<
        ServiceProtocolClientBuilderType>;

      //! Constructs a ServiceLocatorClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      ServiceLocatorClient(ClientBuilderForward&& clientBuilder);

      ~ServiceLocatorClient();

      //! Returns the DirectoryEntry of the account that's logged in.
      DirectoryEntry GetAccount() const;

      //! Returns the raw session id.
      std::string GetSessionId() const;

      //! Encrypts the session id.
      /*!
        \param key The encryption key.
        \return The session id encrypted using the specified encryption
                <i>key</i>.
      */
      std::string GetEncryptedSessionId(unsigned int key) const;

      //! Authenticates an account.
      /*!
        \param name The username of the account.
        \param password The account's password.
        \return DirectoryEntry The account's DirectoryEntry iff the username
                and password match.
      */
      DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password);

      //! Authenticates a session.
      /*!
        \param sessionId An encoded session id.
        \param key The encryption key used to encode the <i>sessionId</i>.
        \return The DirectoryEntry of the account using the specified
               <i>sessionId</i>.
      */
      DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key);

      //! Locates a service.
      /*!
        \param name The name of the service to locate.
        \return The list of services that were located.
      */
      std::vector<ServiceEntry> Locate(const std::string& name);

      //! Registers a service.
      /*!
        \param name The name of the service.
        \param properties The service's properties.
        \return The ServiceEntry corresponding to the registered service.
      */
      ServiceEntry Register(const std::string& name,
        const JsonObject& properties);

      //! Loads the list of all accounts this client is allowed to read.
      std::vector<DirectoryEntry> LoadAllAccounts();

      //! Finds an account with a specified name.
      /*!
        \param name The name of the account to find.
        \return The DirectoryEntry of the account with the given <i>name</i>.
      */
      boost::optional<DirectoryEntry> FindAccount(const std::string& name);

      //! Creates an account.
      /*!
        \param name The name of the account.
        \param password The account's password.
        \param parent The parent to place the account in.
        \return The DirectoryEntry of the account that was created.
      */
      DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent);

      //! Creates a directory.
      /*!
        \param name The name of the directory.
        \param parent The parent to place the directory in.
        \return The DirectoryEntry of the directory that was created.
      */
      DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent);

      //! Sets an account's password.
      /*!
        \param account The account to set the password for.
        \param password The password to set.
      */
      void StorePassword(const DirectoryEntry& account,
        const std::string& password);

      //! Loads a DirectoryEntry from a path.
      /*!
        \param root The root DirectoryEntry to begin searching from.
        \param path The path of the DirectoryEntry to load.
        \return The DirectoryEntry with the specified <i>path</i> from the
                <i>root</i>.
      */
      DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path);

      //! Loads a DirectoryEntry from an id.
      /*!
        \param id The id of the DirectoryEntry to load.
        \return The DirectoryEntry with the specified <i>id</i>.
      */
      DirectoryEntry LoadDirectoryEntry(unsigned int id);

      //! Loads all parents of a specified DirectoryEntry.
      /*!
        \param entry The DirectoryEntry whose parents are to be loaded.
        \return The list of parents of the specified <i>entry</i>.
      */
      std::vector<DirectoryEntry> LoadParents(const DirectoryEntry& entry);

      //! Loads all children of a specified DirectoryEntry.
      /*!
        \param entry The DirectoryEntry whose children are to be loaded.
        \return The list of children of the specified <i>entry</i>.
      */
      std::vector<DirectoryEntry> LoadChildren(const DirectoryEntry& entry);

      //! Deletes a DirectoryEntry.
      /*!
        \param entry The DirectoryEntry to delete.
      */
      void Delete(const DirectoryEntry& entry);

      //! Associates a DirectoryEntry with a parent.
      /*!
        \param entry The DirectoryEntry to associate.
        \param parent The parent to associate with the <i>entry</i>.
      */
      void Associate(const DirectoryEntry& entry, const DirectoryEntry& parent);

      //! Detaches a DirectoryEntry from a parent.
      /*!
        \param entry The DirectoryEntry to detach.
        \param parent The parent to detach from the <i>entry</i>.
      */
      void Detach(const DirectoryEntry& entry, const DirectoryEntry& parent);

      //! Tests if an account has Permissions to a DirectoryEntry.
      /*!
        \param account The account to test.
        \param target The DirectoryEntry to test.
        \param permissions The Permissions to test.
        \return <code>true</code> iff the <i>account</i> has the specified
                <code>permissions</code> on the <i>target</i>.
      */
      bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions);

      //! Sets a DirectoryEntry's Permissions.
      /*!
        \param source The DirectoryEntry to grant the Permissions to.
        \param target The DirectoryEntry to grant the Permissions over.
        \param permissions The Permissions to grant to the <i>source</i> over
               the <i>target</i>.
      */
      void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions);

      //! Loads an account's registration time.
      /*!
        \param account The account whose registration time is to be loaded.
        \return The registration time of the <i>account</i>.
      */
      boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account);

      //! Loads an account's last login time.
      /*!
        \param account The account whose last login time is to be loaded.
        \return The most recent login time of the <i>account</i>.
      */
      boost::posix_time::ptime LoadLastLoginTime(const DirectoryEntry& account);

      //! Renames a DirectoryEntry.
      /*!
        \param entry The DirectoryEntry to rename.
        \param name The name to assign to the DirectoryEntry.
        \return The updated DirectoryEntry.
      */
      DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name);

      //! Sets the credentials used to login to the ServiceLocatorServer.
      /*!
        \param username The username.
        \param password The password.
      */
      void SetCredentials(const std::string& username,
        const std::string& password);

      void Open();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      mutable boost::mutex m_mutex;
      std::string m_username;
      std::string m_password;
      std::string m_sessionId;
      DirectoryEntry m_account;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      IO::OpenState m_openState;

      void Shutdown();
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void Login(ServiceProtocolClient& client);
  };

  //! Loads a directory, or creates it if it doesn't already exist.
  /*!
    \param serviceLocatorClient The ServiceLocatorClient to use.
    \param name The name of the directory to load or create.
    \param parent The directory's parent.
    \return directory The directory that was loaded.
  */
  template<typename ServiceLocatorClient>
  DirectoryEntry LoadOrCreateDirectory(ServiceLocatorClient& client,
      const std::string& name, const DirectoryEntry& parent) {
    try {
      auto entry = client.LoadDirectoryEntry(parent, name);
      return entry;
    } catch(const Services::ServiceRequestException&) {
      return client.MakeDirectory(name, parent);
    }
  }

  //! Locates the IP addresses of a service.
  /*!
    \param client The ServiceLocatorClient used to locate the addresses.
    \param serviceName The name of the service to locate.
    \param servicePredicate A function to apply to a ServiceEntry to determine
           if it matches some criteria.
    \return The list of IP addresses for the specified service.
  */
  template<typename ServiceLocatorClient, typename ServicePredicate>
  std::vector<Network::IpAddress> LocateServiceAddresses(
      ServiceLocatorClient& client, const std::string& serviceName,
      ServicePredicate servicePredicate) {
    std::vector<ServiceEntry> services;
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
    std::random_device seed;
    std::mt19937 generator(seed());
    std::uniform_int_distribution<std::size_t> distribution(
      0, services.size() - 1);
    const auto& service = services[distribution(generator)];
    auto addresses = FromString<std::vector<Network::IpAddress>>(
      boost::get<std::string>(service.GetProperties().At("addresses")));
    return addresses;
  }

  //! Locates the IP addresses of a service.
  /*!
    \param client The ServiceLocatorClient used to locate the addresses.
    \param serviceName The name of the service to locate.
    \return The list of IP addresses for the specified service.
  */
  template<typename ServiceLocatorClient>
  std::vector<Network::IpAddress> LocateServiceAddresses(
      ServiceLocatorClient& client, const std::string& serviceName) {
    return LocateServiceAddresses(client, serviceName,
      [] (auto&) {
        return true;
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  ServiceLocatorClient<ServiceProtocolClientBuilderType>::ServiceLocatorClient(
      ClientBuilderForward&& clientBuilder)
      : m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)) {
    m_clientHandler.SetReconnectHandler(
      std::bind(&ServiceLocatorClient::OnReconnect, this,
      std::placeholders::_1));
    RegisterServiceLocatorServices(Store(m_clientHandler.GetSlots()));
    RegisterServiceLocatorMessages(Store(m_clientHandler.GetSlots()));
  }

  template<typename ServiceProtocolClientBuilderType>
  ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      ~ServiceLocatorClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      GetAccount() const {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    return m_account;
  }

  template<typename ServiceProtocolClientBuilderType>
  std::string ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      GetSessionId() const {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    return m_sessionId;
  }

  template<typename ServiceProtocolClientBuilderType>
  std::string ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      GetEncryptedSessionId(unsigned int key) const {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    return ComputeSHA(ToString(key) + m_sessionId);
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      AuthenticateAccount(const std::string& username,
      const std::string& password) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<AuthenticateAccountService>(username,
      password);
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      AuthenticateSession(const std::string& sessionId, unsigned int key) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<SessionAuthenticationService>(
      sessionId, key);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<ServiceEntry> ServiceLocatorClient<
      ServiceProtocolClientBuilderType>::Locate(const std::string& name) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LocateService>(name);
  }

  template<typename ServiceProtocolClientBuilderType>
  ServiceEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      Register(const std::string& name, const JsonObject& properties) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<RegisterService>(name, properties);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<DirectoryEntry> ServiceLocatorClient<
      ServiceProtocolClientBuilderType>::LoadAllAccounts() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAllAccountsService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  boost::optional<DirectoryEntry> ServiceLocatorClient<
      ServiceProtocolClientBuilderType>::FindAccount(const std::string& name) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<FindAccountService>(name);
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      MakeAccount(const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<MakeAccountService>(name, password,
      parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      MakeDirectory(const std::string& name, const DirectoryEntry& parent) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<MakeDirectoryService>(name, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::StorePassword(
      const DirectoryEntry& account, const std::string& password) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StorePasswordService>(account, password);
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      LoadDirectoryEntry(const DirectoryEntry& root, const std::string& path) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadPathService>(root, path);
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::
      LoadDirectoryEntry(unsigned int id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadDirectoryEntryService>(id);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<DirectoryEntry> ServiceLocatorClient<
      ServiceProtocolClientBuilderType>::LoadParents(
      const DirectoryEntry& entry) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadParentsService>(entry);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<DirectoryEntry> ServiceLocatorClient<
      ServiceProtocolClientBuilderType>::LoadChildren(
      const DirectoryEntry& entry) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadChildrenService>(entry);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::Delete(
      const DirectoryEntry& entry) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<DeleteDirectoryEntryService>(entry);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::Associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<AssociateService>(entry, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::Detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<DetachService>(entry, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  bool ServiceLocatorClient<ServiceProtocolClientBuilderType>::HasPermissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<HasPermissionsService>(account, target,
      permissions);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::StorePermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StorePermissionsService>(source, target,
      permissions);
  }

  template<typename ServiceProtocolClientBuilderType>
  boost::posix_time::ptime ServiceLocatorClient<
      ServiceProtocolClientBuilderType>::LoadRegistrationTime(
      const DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadRegistrationTimeService>(account);
  }

  template<typename ServiceProtocolClientBuilderType>
  boost::posix_time::ptime ServiceLocatorClient<
      ServiceProtocolClientBuilderType>::LoadLastLoginTime(
      const DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadLastLoginTimeService>(account);
  }

  template<typename ServiceProtocolClientBuilderType>
  DirectoryEntry ServiceLocatorClient<ServiceProtocolClientBuilderType>::Rename(
      const DirectoryEntry& entry, const std::string& name) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<RenameService>(entry, name);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::SetCredentials(
      const std::string& username, const std::string& password) {
    m_username = username;
    m_password = password;
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
      auto client = m_clientHandler.GetClient();
      Login(*client);
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    try {
      Login(*client);
    } catch(const std::exception&) {
      Close();
      throw;
    }
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceLocatorClient<ServiceProtocolClientBuilderType>::Login(
      ServiceProtocolClient& client) {
    auto loginResult = client.template SendRequest<LoginService>(m_username,
      m_password);
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_account = loginResult.account;
    m_sessionId = loginResult.session_id;
  }
}
}

#endif
