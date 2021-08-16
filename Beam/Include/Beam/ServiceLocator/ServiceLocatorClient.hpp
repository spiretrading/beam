#ifndef BEAM_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_SERVICE_LOCATOR_CLIENT_HPP
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/ServiceLocator/AccountUpdate.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam::ServiceLocator {

  /**
   * Client used to locate services on a service network.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceLocatorClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = GetTryDereferenceType<B>;

      /**
       * Constructs a ServiceLocatorClient.
       * @param username The username.
       * @param password The password.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      ServiceLocatorClient(std::string username, std::string password,
        BF&& clientBuilder);

      ~ServiceLocatorClient();

      /** Returns the DirectoryEntry of the account that's logged in. */
      DirectoryEntry GetAccount() const;

      /** Returns the raw session id. */
      std::string GetSessionId() const;

      /**
       * Encrypts the session id.
       * @param key The encryption key.
       * @return The session id encrypted using the specified encryption
       *         <i>key</i>.
       */
      std::string GetEncryptedSessionId(unsigned int key) const;

      /**
       * Authenticates an account.
       * @param name The username of the account.
       * @param password The account's password.
       * @return DirectoryEntry The account's DirectoryEntry iff the username
       *         and password match.
       */
      DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password);

      /**
       * Authenticates a session.
       * @param sessionId An encoded session id.
       * @param key The encryption key used to encode the <i>sessionId</i>.
       * @return The DirectoryEntry of the account using the specified
       *        <i>sessionId</i>.
       */
      DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key);

      /**
       * Locates a service.
       * @param name The name of the service to locate.
       * @return The list of services that were located.
       */
      std::vector<ServiceEntry> Locate(const std::string& name);

      /**
       * Registers a service.
       * @param name The name of the service.
       * @param properties The service's properties.
       * @return The ServiceEntry corresponding to the registered service.
       */
      ServiceEntry Register(const std::string& name,
        const JsonObject& properties);

      /**
       * Unregisters a service.
       * @param service The service to unregister.
       */
      void Unregister(const ServiceEntry& service);

      /** Loads the list of all accounts this client is allowed to read. */
      std::vector<DirectoryEntry> LoadAllAccounts();

      /**
       * Finds an account with a specified name.
       * @param name The name of the account to find.
       * @return The DirectoryEntry of the account with the given <i>name</i>.
       */
      boost::optional<DirectoryEntry> FindAccount(const std::string& name);

      /**
       * Creates an account.
       * @param name The name of the account.
       * @param password The account's password.
       * @param parent The parent to place the account in.
       * @return The DirectoryEntry of the account that was created.
       */
      DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent);

      /**
       * Creates a directory.
       * @param name The name of the directory.
       * @param parent The parent to place the directory in.
       * @return The DirectoryEntry of the directory that was created.
       */
      DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent);

      /**
       * Sets an account's password.
       * @param account The account to set the password for.
       * @param password The password to set.
       */
      void StorePassword(const DirectoryEntry& account,
        const std::string& password);

      /**
       * Monitors new and deleted accounts and pushes them to a queue.
       * @param queue The queue to push to.
       */
      void MonitorAccounts(ScopedQueueWriter<AccountUpdate> queue);

      /**
       * Loads a DirectoryEntry from a path.
       * @param root The root DirectoryEntry to begin searching from.
       * @param path The path of the DirectoryEntry to load.
       * @return The DirectoryEntry with the specified <i>path</i> from the
       *         <i>root</i>.
       */
      DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path);

      /**
       * Loads a DirectoryEntry from an id.
       * @param id The id of the DirectoryEntry to load.
       * @return The DirectoryEntry with the specified <i>id</i>.
       */
      DirectoryEntry LoadDirectoryEntry(unsigned int id);

      /**
       * Loads all parents of a specified DirectoryEntry.
       * @param entry The DirectoryEntry whose parents are to be loaded.
       * @return The list of parents of the specified <i>entry</i>.
       */
      std::vector<DirectoryEntry> LoadParents(const DirectoryEntry& entry);

      /**
       * Loads all children of a specified DirectoryEntry.
       * @param entry The DirectoryEntry whose children are to be loaded.
       * @return The list of children of the specified <i>entry</i>.
       */
      std::vector<DirectoryEntry> LoadChildren(const DirectoryEntry& entry);

      /**
       * Deletes a DirectoryEntry.
       * @param entry The DirectoryEntry to delete.
       */
      void Delete(const DirectoryEntry& entry);

      /**
       * Associates a DirectoryEntry with a parent.
       * @param entry The DirectoryEntry to associate.
       * @param parent The parent to associate with the <i>entry</i>.
       */
      void Associate(const DirectoryEntry& entry, const DirectoryEntry& parent);

      /**
       * Detaches a DirectoryEntry from a parent.
       * @param entry The DirectoryEntry to detach.
       * @param parent The parent to detach from the <i>entry</i>.
       */
      void Detach(const DirectoryEntry& entry, const DirectoryEntry& parent);

      /**
       * Tests if an account has Permissions to a DirectoryEntry.
       * @param account The account to test.
       * @param target The DirectoryEntry to test.
       * @param permissions The Permissions to test.
       * @return <code>true</code> iff the <i>account</i> has the specified
       *         <code>permissions</code> on the <i>target</i>.
       */
      bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions);

      /**
       * Sets a DirectoryEntry's Permissions.
       * @param source The DirectoryEntry to grant the Permissions to.
       * @param target The DirectoryEntry to grant the Permissions over.
       * @param permissions The Permissions to grant to the <i>source</i> over
       *        the <i>target</i>.
       */
      void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions);

      /**
       * Loads an account's registration time.
       * @param account The account whose registration time is to be loaded.
       * @return The registration time of the <i>account</i>.
       */
      boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account);

      /**
       * Loads an account's last login time.
       * @param account The account whose last login time is to be loaded.
       * @return The most recent login time of the <i>account</i>.
       */
      boost::posix_time::ptime LoadLastLoginTime(const DirectoryEntry& account);

      /**
       * Renames a DirectoryEntry.
       * @param entry The DirectoryEntry to rename.
       * @param name The name to assign to the DirectoryEntry.
       * @return The updated DirectoryEntry.
       */
      DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name);

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      mutable boost::mutex m_mutex;
      std::string m_username;
      std::string m_password;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      std::string m_sessionId;
      DirectoryEntry m_account;
      std::vector<DirectoryEntry> m_accountUpdateSnapshot;
      QueueWriterPublisher<AccountUpdate> m_accountUpdatePublisher;
      Beam::SynchronizedVector<ServiceEntry> m_services;
      RoutineTaskQueue m_tasks;
      IO::OpenState m_openState;

      ServiceLocatorClient(const ServiceLocatorClient&) = delete;
      ServiceLocatorClient& operator =(const ServiceLocatorClient&) = delete;
      void Login(ServiceProtocolClient& client);
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void OnAccountUpdate(ServiceProtocolClient& client,
        const AccountUpdate& update);
  };

  template<typename B>
  template<typename BF>
  ServiceLocatorClient<B>::ServiceLocatorClient(std::string username,
      std::string password, BF&& clientBuilder)
      try : m_username(std::move(username)),
            m_password(std::move(password)),
            m_clientHandler(std::forward<BF>(clientBuilder),
              std::bind_front(&ServiceLocatorClient::OnReconnect, this)) {
    RegisterServiceLocatorServices(Store(m_clientHandler.GetSlots()));
    RegisterServiceLocatorMessages(Store(m_clientHandler.GetSlots()));
    Services::AddMessageSlot<AccountUpdateMessage>(
      Store(m_clientHandler.GetSlots()),
      std::bind_front(&ServiceLocatorClient::OnAccountUpdate, this));
    try {
      auto client = m_clientHandler.GetClient();
      Login(*client);
    } catch(const std::exception&) {
      Close();
      throw;
    }
  } catch(const std::exception&) {
    std::throw_with_nested(
      IO::ConnectException("Failed to login to service locator."));
  }

  template<typename B>
  ServiceLocatorClient<B>::~ServiceLocatorClient() {
    Close();
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::GetAccount() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account;
  }

  template<typename B>
  std::string ServiceLocatorClient<B>::GetSessionId() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_sessionId;
  }

  template<typename B>
  std::string ServiceLocatorClient<B>::GetEncryptedSessionId(
      unsigned int key) const {
    auto lock = boost::lock_guard(m_mutex);
    return ComputeSHA(std::to_string(key) + m_sessionId);
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::AuthenticateAccount(
      const std::string& username, const std::string& password) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<AuthenticateAccountService>(username,
        password);
    }, "Error authenticating account: " + username);
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::
      AuthenticateSession(const std::string& sessionId, unsigned int key) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<SessionAuthenticationService>(
        sessionId, key);
    }, "Error authenticating session: (" + sessionId + ", " +
      std::to_string(key) + ")");
  }

  template<typename B>
  std::vector<ServiceEntry> ServiceLocatorClient<B>::Locate(
      const std::string& name) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LocateService>(name);
    }, "Error locating service: " + name);
  }

  template<typename B>
  ServiceEntry ServiceLocatorClient<B>::Register(const std::string& name,
      const JsonObject& properties) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      auto service = client->template SendRequest<RegisterService>(name,
        properties);
      m_services.PushBack(service);
      return service;
    }, "Error registering service: " + name);
  }

  template<typename B>
  void ServiceLocatorClient<B>::Unregister(const ServiceEntry& service) {
    Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<UnregisterService>(service.GetId());
      m_services.RemoveIf([&] (const auto& s) {
        return s.GetId() == service.GetId();
      });
    }, "Error unregistering service: " + service.GetName());
  }

  template<typename B>
  std::vector<DirectoryEntry> ServiceLocatorClient<B>::LoadAllAccounts() {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadAllAccountsService>();
    }, "Error loading all accounts.");
  }

  template<typename B>
  boost::optional<DirectoryEntry> ServiceLocatorClient<B>::FindAccount(
      const std::string& name) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<FindAccountService>(name);
    }, "Error finding account: " + name);
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::MakeAccount(const std::string& name,
      const std::string& password, const DirectoryEntry& parent) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<MakeAccountService>(name, password,
        parent);
    }, "Error making account: " + name);
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::MakeDirectory(const std::string& name,
      const DirectoryEntry& parent) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<MakeDirectoryService>(name, parent);
    }, "Error making directory: " + name);
  }

  template<typename B>
  void ServiceLocatorClient<B>::StorePassword(const DirectoryEntry& account,
      const std::string& password) {
    Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<StorePasswordService>(account, password);
    }, "Error storing password for account: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ServiceLocatorClient<B>::MonitorAccounts(
      ScopedQueueWriter<AccountUpdate> queue) {
    m_tasks.Push([this, queue =
        std::make_shared<ScopedQueueWriter<AccountUpdate>>(std::move(queue))] {
      if(m_accountUpdatePublisher.GetSize() != 0) {
        try {
          for(auto& account : m_accountUpdateSnapshot) {
            queue->Push(AccountUpdate{account, AccountUpdate::Type::ADDED});
          }
          m_accountUpdatePublisher.Monitor(std::move(*queue));
        } catch(const PipeBrokenException&) {}
        return;
      }
      try {
        auto client = m_clientHandler.GetClient();
        m_accountUpdateSnapshot =
          client->template SendRequest<MonitorAccountsService>();
      } catch(const std::exception&) {
        queue->Break(Services::MakeNestedServiceException(
          "Monitor accounts failed."));
        return;
      }
      m_accountUpdatePublisher.Monitor(std::move(*queue));
      for(auto& account : m_accountUpdateSnapshot) {
        m_accountUpdatePublisher.Push(
          AccountUpdate{account, AccountUpdate::Type::ADDED});
      }
    });
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::LoadDirectoryEntry(
      const DirectoryEntry& root, const std::string& path) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadPathService>(root, path);
    }, "Error loading directory entry path: " +
      boost::lexical_cast<std::string>(root) + ", " + path);
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::LoadDirectoryEntry(unsigned int id) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadDirectoryEntryService>(id);
    }, "Error loading directory entry: " + std::to_string(id));
  }

  template<typename B>
  std::vector<DirectoryEntry> ServiceLocatorClient<B>::LoadParents(
      const DirectoryEntry& entry) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadParentsService>(entry);
    }, "Error loading parents: " + boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  std::vector<DirectoryEntry> ServiceLocatorClient<B>::LoadChildren(
      const DirectoryEntry& entry) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadChildrenService>(entry);
    }, "Error loading children: " + boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  void ServiceLocatorClient<B>::Delete(const DirectoryEntry& entry) {
    Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<DeleteDirectoryEntryService>(entry);
    }, "Error deleting: " + boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  void ServiceLocatorClient<B>::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<AssociateService>(entry, parent);
    }, "Error associating: " + boost::lexical_cast<std::string>(entry) + ", " +
      boost::lexical_cast<std::string>(parent));
  }

  template<typename B>
  void ServiceLocatorClient<B>::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<DetachService>(entry, parent);
    }, "Error detaching: " + boost::lexical_cast<std::string>(entry) + ", " +
      boost::lexical_cast<std::string>(parent));
  }

  template<typename B>
  bool ServiceLocatorClient<B>::HasPermissions(const DirectoryEntry& account,
      const DirectoryEntry& target, Permissions permissions) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<HasPermissionsService>(account,
        target, permissions);
    }, "Error checking permissions: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(target));
  }

  template<typename B>
  void ServiceLocatorClient<B>::StorePermissions(const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<StorePermissionsService>(source, target,
        permissions);
    }, "Error storing permissions: " +
      boost::lexical_cast<std::string>(source) + ", " +
      boost::lexical_cast<std::string>(target));
  }

  template<typename B>
  boost::posix_time::ptime ServiceLocatorClient<B>::LoadRegistrationTime(
      const DirectoryEntry& account) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadRegistrationTimeService>(account);
    }, "Error loading registration time: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  boost::posix_time::ptime ServiceLocatorClient<B>::LoadLastLoginTime(
      const DirectoryEntry& account) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadLastLoginTimeService>(account);
    }, "Error loading last login time: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  DirectoryEntry ServiceLocatorClient<B>::Rename(const DirectoryEntry& entry,
      const std::string& name) {
    return Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<RenameService>(entry, name);
    }, "Error renaming: " + boost::lexical_cast<std::string>(entry) + ", " +
      name);
  }

  template<typename B>
  void ServiceLocatorClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_accountUpdatePublisher.Break();
    m_tasks.Break();
    m_tasks.Wait();
    m_clientHandler.Close();
    m_openState.Close();
  }

  template<typename B>
  void ServiceLocatorClient<B>::Login(ServiceProtocolClient& client) {
    auto loginResult = client.template SendRequest<LoginService>(m_username,
      m_password);
    auto lock = boost::lock_guard(m_mutex);
    m_account = loginResult.account;
    m_sessionId = loginResult.session_id;
  }

  template<typename B>
  void ServiceLocatorClient<B>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    Login(*client);
    auto services = std::vector<ServiceEntry>();
    m_services.Swap(services);
    m_tasks.Push([=, this] {
      for(auto& service : services) {
        Register(service.GetName(), service.GetProperties());
      }
      if(m_accountUpdatePublisher.GetSize() == 0) {
        return;
      }
      auto client = m_clientHandler.GetClient();
      auto accounts = client->template SendRequest<MonitorAccountsService>();
      for(auto& account : accounts) {
        auto i = std::find(m_accountUpdateSnapshot.begin(),
          m_accountUpdateSnapshot.end(), account);
        if(i == m_accountUpdateSnapshot.end()) {
          m_accountUpdateSnapshot.push_back(account);
          m_accountUpdatePublisher.Push(
            AccountUpdate{account, AccountUpdate::Type::ADDED});
        }
      }
    });
  }

  template<typename B>
  void ServiceLocatorClient<B>::OnAccountUpdate(ServiceProtocolClient& client,
      const AccountUpdate& update) {
    m_tasks.Push([=, this] {
      if(update.m_type == AccountUpdate::Type::ADDED) {
        m_accountUpdateSnapshot.push_back(update.m_account);
      } else {
        auto i = std::find(m_accountUpdateSnapshot.begin(),
          m_accountUpdateSnapshot.end(), update.m_account);
        if(i != m_accountUpdateSnapshot.end()) {
          m_accountUpdateSnapshot.erase(i);
        }
      }
      m_accountUpdatePublisher.Push(update);
      if(m_accountUpdatePublisher.GetSize() == 0) {
        auto client = m_clientHandler.GetClient();
        client->template SendRequest<UnmonitorAccountsService>();
        m_accountUpdateSnapshot = {};
      }
    });
  }
}

#endif
