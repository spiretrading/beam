#ifndef BEAM_SERVICE_LOCATOR_SERVLET_HPP
#define BEAM_SERVICE_LOCATOR_SERVLET_HPP
#include <algorithm>
#include <atomic>
#include <random>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/ServiceLocatorSession.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam::ServiceLocator {

  /**
   * Handles ServiceLocator service requests.
   * @param <C> The container instantiating this servlet.
   * @param <D> The type of data store to use.
   */
  template<typename C, typename D>
  class ServiceLocatorServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of ServiceLocatorDataStore used. */
      using ServiceLocatorDataStore = GetTryDereferenceType<D>;

      /**
       * Constructs a ServiceLocatorServlet.
       * @param dataStore The data store to use.
       */
      template<typename DF>
      explicit ServiceLocatorServlet(DF&& dataStore);

      void RegisterServices(
        Out<Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      struct ServiceEntryListing {
        std::vector<ServiceEntry> m_entries;
        std::vector<ServiceProtocolClient*> m_subscribers;
      };
      struct DirectoryEntryMonitorEntry {
        std::vector<ServiceProtocolClient*> m_subscribers;
      };
      using ServiceEntryListings =
        std::unordered_map<std::string, ServiceEntryListing>;
      using ServiceListings = std::unordered_map<int, ServiceEntry>;
      using DirectoryEntryMonitorEntries =
        std::unordered_map<DirectoryEntry, DirectoryEntryMonitorEntry>;
      using Sessions = std::unordered_map<std::string, ServiceProtocolClient*>;
      GetOptionalLocalPtr<D> m_dataStore;
      SynchronizedVector<ServiceProtocolClient*> m_accountUpdateSubscribers;
      Threading::Sync<Sessions> m_sessions;
      Threading::Sync<ServiceListings> m_serviceListings;
      Threading::Sync<ServiceEntryListings> m_serviceEntryListings;
      Threading::Sync<DirectoryEntryMonitorEntries>
        m_directoryEntryMonitorEntries;
      std::atomic_int m_nextServiceId;
      IO::OpenState m_openState;

      ServiceLocatorServlet(const ServiceLocatorServlet&) = delete;
      ServiceLocatorServlet& operator =(
        const ServiceLocatorServlet&) = delete;
      void Delete(const DirectoryEntry& entry);
      LoginServiceResult OnLoginRequest(ServiceProtocolClient& client,
        const std::string& username, const std::string& password);
      ServiceEntry OnRegisterRequest(ServiceProtocolClient& client,
        const std::string& name, const JsonObject& properties);
      void OnUnregisterRequest(ServiceProtocolClient& client, int serviceId);
      std::vector<ServiceEntry> OnLocateRequest(ServiceProtocolClient& client,
        const std::string& name);
      std::vector<ServiceEntry> OnSubscribeRequest(
        ServiceProtocolClient& client, const std::string& serviceName);
      void OnUnsubscribeRequest(ServiceProtocolClient& client,
        const std::string& serviceName);
      std::vector<DirectoryEntry> OnMonitorDirectoryEntryRequest(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      DirectoryEntry OnLoadPath(ServiceProtocolClient& client,
        const DirectoryEntry& root, const std::string& path);
      std::vector<DirectoryEntry> OnMonitorAccounts(
        ServiceProtocolClient& client);
      void OnUnmonitorAccounts(ServiceProtocolClient& client);
      DirectoryEntry OnLoadDirectoryEntry(ServiceProtocolClient& client,
        unsigned int id);
      std::vector<DirectoryEntry> OnLoadParentsRequest(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      std::vector<DirectoryEntry> OnLoadChildrenRequest(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      std::vector<DirectoryEntry> OnLoadAllAccountsRequest(
        ServiceProtocolClient& client);
      boost::optional<DirectoryEntry> OnFindAccountRequest(
        ServiceProtocolClient& client, const std::string& name);
      DirectoryEntry OnMakeAccountRequest(ServiceProtocolClient& client,
        const std::string& name, const std::string& password,
        const DirectoryEntry& parent);
      DirectoryEntry OnMakeDirectoryRequest(ServiceProtocolClient& client,
        const std::string& name, const DirectoryEntry& parent);
      void OnDeleteDirectoryEntryRequest(ServiceProtocolClient& client,
        const DirectoryEntry& entry);
      void OnAssociateRequest(ServiceProtocolClient& client,
        const DirectoryEntry& entry, const DirectoryEntry& parent);
      void OnDetachRequest(ServiceProtocolClient& client,
        const DirectoryEntry& entry, const DirectoryEntry& parent);
      void OnStorePasswordRequest(ServiceProtocolClient& client,
        const DirectoryEntry& account, const std::string& password);
      bool OnHasPermissionsRequest(ServiceProtocolClient& client,
        const DirectoryEntry& account, const DirectoryEntry& target,
        Permissions permissions);
      void OnStorePermissionsRequest(ServiceProtocolClient& client,
        const DirectoryEntry& source, const DirectoryEntry& target,
        Permissions permissions);
      boost::posix_time::ptime OnLoadRegistrationTimeRequest(
        ServiceProtocolClient& client, const DirectoryEntry& account);
      boost::posix_time::ptime OnLoadLastLoginTimeRequest(
        ServiceProtocolClient& client, const DirectoryEntry& account);
      DirectoryEntry OnRenameRequest(ServiceProtocolClient& client,
        const DirectoryEntry& entry, const std::string& name);
      DirectoryEntry OnAuthenticateAccountRequest(
        ServiceProtocolClient& client, const std::string& username,
        const std::string& password);
      DirectoryEntry OnSessionAuthenticationRequest(
        ServiceProtocolClient& client, const std::string& sessionId,
        unsigned int saltId);
  };

  template<typename D>
  struct MetaServiceLocatorServlet {
    static constexpr auto SupportsParallelism = true;
    using Session = ServiceLocatorSession;
    template<typename C>
    struct apply {
      using type = ServiceLocatorServlet<C, D>;
    };
  };

  template<typename C, typename D>
  template<typename DF>
  ServiceLocatorServlet<C, D>::ServiceLocatorServlet(DF&& dataStore)
      : m_dataStore(std::forward<DF>(dataStore)) {
    try {
      m_nextServiceId = 1;
      m_dataStore->WithTransaction([&] {
        try {
          m_dataStore->LoadDirectoryEntry(0);
          return;
        } catch(const ServiceLocatorDataStoreException&) {}
        auto starDirectory = m_dataStore->MakeDirectory("*",
          DirectoryEntry::MakeDirectory(static_cast<unsigned int>(-1), "*"));
        auto rootAccount = m_dataStore->MakeAccount("root", "", starDirectory,
          boost::posix_time::second_clock::universal_time());
        m_dataStore->SetPermissions(rootAccount, starDirectory,
          static_cast<Permissions>(~0));
      });
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::RegisterServices(
      Out<Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterServiceLocatorServices(Store(slots));
    RegisterServiceLocatorMessages(Store(slots));
    LoginService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnLoginRequest, this));
    RegisterService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnRegisterRequest, this));
    UnregisterService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnUnregisterRequest, this));
    LocateService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnLocateRequest, this));
    SubscribeAvailabilityService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnSubscribeRequest, this));
    UnsubscribeAvailabilityService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnUnsubscribeRequest, this));
    MonitorDirectoryEntryService::AddSlot(Store(slots), std::bind_front(
      &ServiceLocatorServlet::OnMonitorDirectoryEntryRequest, this));
    LoadPathService::AddSlot(
      Store(slots), std::bind_front(&ServiceLocatorServlet::OnLoadPath, this));
    MonitorAccountsService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnMonitorAccounts, this));
    UnmonitorAccountsService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnUnmonitorAccounts, this));
    LoadDirectoryEntryService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnLoadDirectoryEntry, this));
    LoadParentsService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnLoadParentsRequest, this));
    LoadChildrenService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnLoadChildrenRequest, this));
    LoadAllAccountsService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnLoadAllAccountsRequest, this));
    FindAccountService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnFindAccountRequest, this));
    MakeAccountService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnMakeAccountRequest, this));
    MakeDirectoryService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnMakeDirectoryRequest, this));
    DeleteDirectoryEntryService::AddSlot(Store(slots), std::bind_front(
      &ServiceLocatorServlet::OnDeleteDirectoryEntryRequest, this));
    AssociateService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnAssociateRequest, this));
    DetachService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnDetachRequest, this));
    StorePasswordService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnStorePasswordRequest, this));
    HasPermissionsService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnHasPermissionsRequest, this));
    StorePermissionsService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnStorePermissionsRequest, this));
    LoadRegistrationTimeService::AddSlot(Store(slots),std::bind_front(
      &ServiceLocatorServlet::OnLoadRegistrationTimeRequest, this));
    LoadLastLoginTimeService::AddSlot(Store(slots), std::bind_front(
      &ServiceLocatorServlet::OnLoadLastLoginTimeRequest, this));
    RenameService::AddSlot(Store(slots),
      std::bind_front(&ServiceLocatorServlet::OnRenameRequest, this));
    AuthenticateAccountService::AddSlot(Store(slots), std::bind_front(
      &ServiceLocatorServlet::OnAuthenticateAccountRequest, this));
    SessionAuthenticationService::AddSlot(Store(slots), std::bind_front(
      &ServiceLocatorServlet::OnSessionAuthenticationRequest, this));
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::HandleClientClosed(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    auto registeredServices = session.GetRegisteredServices();
    auto serviceSubscriptions = session.GetServiceSubscriptions();
    Threading::With(m_serviceEntryListings, [&] (auto& serviceEntryListings) {
      for(auto& registeredService : registeredServices) {
        auto& listing = serviceEntryListings[registeredService.GetName()];
        RemoveAll(listing.m_entries, registeredService);
        Services::BroadcastRecordMessage<ServiceAvailabilityMessage>(
          listing.m_subscribers, registeredService, false);
      }
      for(auto& serviceSubscription : serviceSubscriptions) {
        auto& listing = serviceEntryListings[serviceSubscription];
        RemoveAll(listing.m_subscribers, &client);
      }
    });
    auto monitors = session.GetMonitors();
    Threading::With(m_directoryEntryMonitorEntries,
      [&] (auto& directoryEntryMonitorEntries) {
        for(auto& entry : monitors) {
          auto& monitor = directoryEntryMonitorEntries[entry];
          RemoveAll(monitor.m_subscribers, &client);
        }
      });
    Threading::With(m_sessions, [&] (auto& sessions) {
      sessions.erase(session.GetSessionId());
    });
    m_accountUpdateSubscribers.Remove(&client);
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_openState.Close();
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::Delete(const DirectoryEntry& entry) {
    auto children = m_dataStore->LoadChildren(entry);
    for(auto& child : children) {
      auto parents = m_dataStore->LoadParents(child);
      if(parents.size() == 1) {
        Delete(child);
      } else {
        m_dataStore->Detach(child, entry);
      }
    }
    auto parents = m_dataStore->LoadParents(entry);
    if(entry.m_type == DirectoryEntry::Type::ACCOUNT) {
      m_accountUpdateSubscribers.ForEach([&] (auto& subscriber) {
        if(HasPermission(*m_dataStore, subscriber->GetSession().GetAccount(),
            entry, Permission::READ)) {
          Services::SendRecordMessage<AccountUpdateMessage>(*subscriber,
            AccountUpdate{entry, AccountUpdate::Type::DELETED});
        }
      });
    }
    m_dataStore->Delete(entry);
    Threading::With(m_directoryEntryMonitorEntries,
      [&] (auto& directoryEntryMonitorEntries) {
        auto monitorIterator = directoryEntryMonitorEntries.find(entry);
        if(monitorIterator == directoryEntryMonitorEntries.end()) {
          return;
        }
        auto& monitor = monitorIterator->second;
        for(auto& parent : parents) {
          Services::BroadcastRecordMessage<DirectoryEntryDetachedMessage>(
            monitor.m_subscribers, entry, parent);
        }
        directoryEntryMonitorEntries.erase(monitorIterator);
      });
  }

  template<typename C, typename D>
  LoginServiceResult ServiceLocatorServlet<C, D>::OnLoginRequest(
      ServiceProtocolClient& client, const std::string& username,
      const std::string& password) {
    auto& session = client.GetSession();
    if(!session.TryLogin()) {
      throw Services::ServiceRequestException("Account is already logged in.");
    }
    auto account = DirectoryEntry();
    try {
      m_dataStore->WithTransaction([&] {
        try {
          account = m_dataStore->LoadAccount(username);
        } catch(const ServiceLocatorDataStoreException&) {
          throw Services::ServiceRequestException(
            "Invalid username or password.");
        }
        auto accountPassword = std::string();
        try {
          accountPassword = m_dataStore->LoadPassword(account);
        } catch(const ServiceLocatorDataStoreException&) {
          throw Services::ServiceRequestException(
            "Unable to retrieve password, try again later.");
        }
        if(!ValidatePassword(account, password, accountPassword)) {
          throw Services::ServiceRequestException(
            "Invalid username or password.");
        }
        m_dataStore->StoreLastLoginTime(account,
          boost::posix_time::second_clock::universal_time());
      });
    } catch(const std::exception&) {
      session.ResetLogin();
      throw;
    }
    auto sessionId = std::string();
    Threading::With(m_sessions, [&] (auto& sessions) {
      do {
        sessionId = GenerateSessionId();
      } while(sessions.find(sessionId) != sessions.end());
      sessions.insert(std::make_pair(sessionId, &client));
    });
    session.SetSessionId(account, sessionId);
    return LoginServiceResult(account, sessionId);
  }

  template<typename C, typename D>
  ServiceEntry ServiceLocatorServlet<C, D>::OnRegisterRequest(
      ServiceProtocolClient& client, const std::string& name,
      const JsonObject& properties) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto id = ++m_nextServiceId;
    auto entry = ServiceEntry(name, properties, id, session.GetAccount());
    Threading::With(m_serviceEntryListings, m_serviceListings,
      [&] (auto& serviceEntryListings, auto& serviceListings) {
        auto& listing = serviceEntryListings[name];
        listing.m_entries.push_back(entry);
        serviceListings.insert(std::make_pair(id, entry));
        session.RegisterService(entry);
        Services::BroadcastRecordMessage<ServiceAvailabilityMessage>(
          listing.m_subscribers, entry, true);
      });
    return entry;
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnUnregisterRequest(
      ServiceProtocolClient& client, int serviceId) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto registeredServices = session.GetRegisteredServices();
    auto serviceFound = false;
    for(auto& registeredService : registeredServices) {
      if(registeredService.GetId() == serviceId) {
        serviceFound = true;
        break;
      }
    }
    if(!serviceFound) {
      throw Services::ServiceRequestException("Service not found.");
    }
    Threading::With(m_serviceEntryListings, m_serviceListings,
      [&] (auto& serviceEntryListings, auto& serviceListings) {
        auto& entry = serviceListings[serviceId];
        auto& listing = serviceEntryListings[entry.GetName()];
        RemoveAll(listing.m_entries, entry);
        session.UnregisterService(serviceId);
        Services::BroadcastRecordMessage<ServiceAvailabilityMessage>(
          listing.m_subscribers, entry, false);
      });
  }

  template<typename C, typename D>
  std::vector<ServiceEntry> ServiceLocatorServlet<C, D>::OnLocateRequest(
      ServiceProtocolClient& client, const std::string& name) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto listings = Threading::With(m_serviceEntryListings,
      [&] (auto& serviceEntryListings) -> std::vector<ServiceEntry> {
        auto entryIterator = serviceEntryListings.find(name);
        if(entryIterator != serviceEntryListings.end()) {
          return entryIterator->second.m_entries;
        }
        return {};
      });
    auto randomDevice = std::random_device();
    auto randomGenerator = std::mt19937(randomDevice());
    std::shuffle(listings.begin(), listings.end(), randomGenerator);
    return listings;
  }

  template<typename C, typename D>
  std::vector<ServiceEntry> ServiceLocatorServlet<C, D>::OnSubscribeRequest(
      ServiceProtocolClient& client, const std::string& serviceName) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    return Threading::With(m_serviceEntryListings,
      [&] (auto& serviceEntryListings) -> std::vector<ServiceEntry> {
        auto& listing = serviceEntryListings[serviceName];
        if(std::find(session.GetServiceSubscriptions().begin(),
            session.GetServiceSubscriptions().end(), serviceName) !=
            session.GetServiceSubscriptions().end()) {
          return {};
        }
        listing.m_subscribers.push_back(&client);
        session.SubscribeService(serviceName);
        return listing.m_entries;
      });
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnUnsubscribeRequest(
      ServiceProtocolClient& client, const std::string& serviceName) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    Threading::With(m_serviceEntryListings, [&] (auto& serviceEntryListings) {
      auto& listing = serviceEntryListings[serviceName];
      RemoveAll(listing.m_subscribers, &client);
      session.UnsubscribeService(serviceName);
    });
  }

  template<typename C, typename D>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::
      OnMonitorDirectoryEntryRequest(ServiceProtocolClient& client,
      const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto parents = std::vector<DirectoryEntry>();
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(entry);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
          Permission::READ)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      parents = m_dataStore->LoadParents(validatedEntry);
    });
    Threading::With(m_directoryEntryMonitorEntries,
      [&] (auto& directoryEntryMonitorEntries) {
        auto& monitor = directoryEntryMonitorEntries[entry];
        if(std::find(monitor.m_subscribers.begin(), monitor.m_subscribers.end(),
            &client) != monitor.m_subscribers.end()) {
          throw Services::ServiceRequestException("Already subscribed.");
        }
        monitor.m_subscribers.push_back(&client);
        session.Monitor(entry);
      });
    return parents;
  }

  template<typename C, typename D>
  DirectoryEntry ServiceLocatorServlet<C, D>::OnLoadPath(
      ServiceProtocolClient& client, const DirectoryEntry& root,
      const std::string& path) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto entry = DirectoryEntry();
    m_dataStore->WithTransaction([&] {
      entry = LoadDirectoryEntry(*m_dataStore, root, path);
      if(!HasPermission(*m_dataStore, session.GetAccount(), entry,
          Permission::READ)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
    });
    return entry;
  }

  template<typename C, typename D>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::OnMonitorAccounts(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto accounts = std::vector<DirectoryEntry>();
    m_dataStore->WithTransaction([&] {
      accounts = m_accountUpdateSubscribers.With(
        [&] (auto& accountUpdateSubscribers) {
          auto i = std::find(accountUpdateSubscribers.begin(),
            accountUpdateSubscribers.end(), &client);
          if(i == accountUpdateSubscribers.end()) {
            accountUpdateSubscribers.push_back(&client);
          }
          auto accounts = m_dataStore->LoadAllAccounts();
          accounts.erase(std::remove_if(accounts.begin(), accounts.end(),
            [&] (auto& account) {
              return !HasPermission(*m_dataStore, session.GetAccount(), account,
                Permission::READ);
            }), accounts.end());
          return accounts;
        });
      });
    return accounts;
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnUnmonitorAccounts(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    m_accountUpdateSubscribers.Remove(&client);
  }

  template<typename C, typename D>
  DirectoryEntry ServiceLocatorServlet<C, D>::OnLoadDirectoryEntry(
      ServiceProtocolClient& client, unsigned int id) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto entry = DirectoryEntry();
    m_dataStore->WithTransaction([&] {
      entry = m_dataStore->LoadDirectoryEntry(id);
      if(!HasPermission(*m_dataStore, session.GetAccount(), entry,
          Permission::READ)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
    });
    return entry;
  }

  template<typename C, typename D>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::OnLoadParentsRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto parents = std::vector<DirectoryEntry>();
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(entry);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
          Permission::READ)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      parents = m_dataStore->LoadParents(validatedEntry);
    });
    return parents;
  }

  template<typename C, typename D>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::
      OnLoadChildrenRequest(ServiceProtocolClient& client,
      const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto children = std::vector<DirectoryEntry>();
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(entry);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
          Permission::READ)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      children = m_dataStore->LoadChildren(validatedEntry);
    });
    return children;
  }

  template<typename C, typename D>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::
      OnLoadAllAccountsRequest(ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto accounts = std::vector<DirectoryEntry>();
    m_dataStore->WithTransaction([&] {
      accounts = m_dataStore->LoadAllAccounts();
      accounts.erase(std::remove_if(accounts.begin(), accounts.end(),
        [&] (auto& account) {
          return !HasPermission(*m_dataStore, session.GetAccount(), account,
            Permission::READ);
        }), accounts.end());
    });
    return accounts;
  }

  template<typename C, typename D>
  boost::optional<DirectoryEntry> ServiceLocatorServlet<C, D>::
      OnFindAccountRequest(ServiceProtocolClient& client,
      const std::string& name) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto account = boost::optional<DirectoryEntry>();
    m_dataStore->WithTransaction([&] {
      try {
        account = m_dataStore->LoadAccount(name);
      } catch(const ServiceLocatorDataStoreException&) {}
    });
    return account;
  }

  template<typename C, typename D>
  DirectoryEntry ServiceLocatorServlet<C, D>::OnMakeAccountRequest(
      ServiceProtocolClient& client, const std::string& name,
      const std::string& password, const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto validatedName = boost::trim_copy(name);
    if(validatedName.empty()) {
      throw Services::ServiceRequestException("Name is empty.");
    }
    auto newEntry = DirectoryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedParent = m_dataStore->Validate(parent);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
          Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      newEntry = m_dataStore->MakeAccount(validatedName, password,
        validatedParent, boost::posix_time::second_clock::universal_time());
      m_accountUpdateSubscribers.ForEach([&] (auto& subscriber) {
        if(HasPermission(*m_dataStore, subscriber->GetSession().GetAccount(),
            newEntry, Permission::READ)) {
          Services::SendRecordMessage<AccountUpdateMessage>(*subscriber,
            AccountUpdate{newEntry, AccountUpdate::Type::ADDED});
        }
      });
    });
    return newEntry;
  }

  template<typename C, typename D>
  DirectoryEntry ServiceLocatorServlet<C, D>::OnMakeDirectoryRequest(
      ServiceProtocolClient& client, const std::string& name,
      const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto validatedName = boost::trim_copy(name);
    if(validatedName.empty()) {
      throw Services::ServiceRequestException("Name is empty.");
    }
    auto newEntry = DirectoryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedParent = m_dataStore->Validate(parent);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
          Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      newEntry = m_dataStore->MakeDirectory(validatedName, validatedParent);
    });
    return newEntry;
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnDeleteDirectoryEntryRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(entry);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
          Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      Delete(validatedEntry);
    });
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnAssociateRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(entry);
      auto validatedParent = m_dataStore->Validate(parent);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
          Permission::MOVE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      if(!m_dataStore->Associate(validatedEntry, validatedParent)) {
        return;
      }
      Threading::With(m_directoryEntryMonitorEntries,
        [&] (auto& directoryEntryMonitorEntries) {
          auto monitorIterator = directoryEntryMonitorEntries.find(
            validatedEntry);
          if(monitorIterator == directoryEntryMonitorEntries.end()) {
            return;
          }
          auto& monitor = monitorIterator->second;
          Services::BroadcastRecordMessage<DirectoryEntryAssociatedMessage>(
            monitor.m_subscribers, validatedEntry, validatedParent);
        });
    });
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnDetachRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(entry);
      auto validatedParent = m_dataStore->Validate(parent);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
          Permission::MOVE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      auto parents = m_dataStore->LoadParents(validatedEntry);
      if(parents.size() == 1) {
        throw Services::ServiceRequestException(
          "Entry only has one parent, must be deleted instead of detached.");
      }
      if(!m_dataStore->Detach(validatedEntry, validatedParent)) {
        return;
      }
      Threading::With(m_directoryEntryMonitorEntries,
        [&] (auto& directoryEntryMonitorEntries) {
          auto monitorIterator = directoryEntryMonitorEntries.find(
            validatedEntry);
          if(monitorIterator == directoryEntryMonitorEntries.end()) {
            return;
          }
          auto& monitor = monitorIterator->second;
          Services::BroadcastRecordMessage<DirectoryEntryDetachedMessage>(
            monitor.m_subscribers, validatedEntry, validatedParent);
        });
    });
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnStorePasswordRequest(
      ServiceProtocolClient& client, const DirectoryEntry& account,
      const std::string& password) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    m_dataStore->WithTransaction([&] {
      auto validatedAccount = m_dataStore->Validate(account);
      if(validatedAccount != session.GetAccount() &&
          !HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
          Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      m_dataStore->SetPassword(validatedAccount,
        HashPassword(validatedAccount, password));
    });
  }

  template<typename C, typename D>
  bool ServiceLocatorServlet<C, D>::OnHasPermissionsRequest(
      ServiceProtocolClient& client, const DirectoryEntry& account,
      const DirectoryEntry& target, Permissions permissions) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto result = false;
    m_dataStore->WithTransaction([&] {
      auto validatedAccount = m_dataStore->Validate(account);
      auto validatedTarget = m_dataStore->Validate(target);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
          Permission::ADMINISTRATE) || !HasPermission(*m_dataStore,
          session.GetAccount(), validatedTarget, Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      result = HasPermission(*m_dataStore, account, target, permissions);
    });
    return result;
  }

  template<typename C, typename D>
  void ServiceLocatorServlet<C, D>::OnStorePermissionsRequest(
      ServiceProtocolClient& client, const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    m_dataStore->WithTransaction([&] {
      auto validatedSource = m_dataStore->Validate(source);
      auto validatedTarget = m_dataStore->Validate(target);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedSource,
          Permission::ADMINISTRATE) ||
          !HasPermission(*m_dataStore, session.GetAccount(), validatedTarget,
          Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      m_dataStore->SetPermissions(source, validatedTarget, permissions);
    });
  }

  template<typename C, typename D>
  boost::posix_time::ptime ServiceLocatorServlet<C, D>::
      OnLoadRegistrationTimeRequest(ServiceProtocolClient& client,
      const DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto registrationTime = boost::posix_time::ptime();
    m_dataStore->WithTransaction([&] {
      auto validatedAccount = m_dataStore->Validate(account);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
          Permission::READ)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      registrationTime = m_dataStore->LoadRegistrationTime(validatedAccount);
    });
    return registrationTime;
  }

  template<typename C, typename D>
  boost::posix_time::ptime ServiceLocatorServlet<C, D>::
      OnLoadLastLoginTimeRequest(ServiceProtocolClient& client,
      const DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto lastLoginTime = boost::posix_time::ptime();
    m_dataStore->WithTransaction([&] {
      auto validatedAccount = m_dataStore->Validate(account);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
          Permission::READ)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      lastLoginTime = m_dataStore->LoadLastLoginTime(validatedAccount);
    });
    return lastLoginTime;
  }

  template<typename C, typename D>
  DirectoryEntry ServiceLocatorServlet<C, D>::OnRenameRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry,
      const std::string& name) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto result = DirectoryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(entry);
      if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
          Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      auto isExistingAccount = false;
      try {
        m_dataStore->LoadAccount(name);
        isExistingAccount = true;
      } catch(const ServiceLocatorDataStoreException&) {
        isExistingAccount = false;
      }
      if(isExistingAccount) {
        throw Services::ServiceRequestException(
          "An account with the specified name exists.");
      }
      m_dataStore->Rename(validatedEntry, name);
      result = validatedEntry;
      result.m_name = name;
    });
    return result;
  }

  template<typename C, typename D>
  DirectoryEntry ServiceLocatorServlet<C, D>::OnAuthenticateAccountRequest(
      ServiceProtocolClient& client, const std::string& username,
      const std::string& password) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto account = DirectoryEntry();
    m_dataStore->WithTransaction([&] {
      auto entry = DirectoryEntry();
      try {
        entry = m_dataStore->LoadAccount(username);
      } catch(const ServiceLocatorDataStoreException&) {
        return;
      }
      if(!HasPermission(*m_dataStore, session.GetAccount(), entry,
          Permission::ADMINISTRATE)) {
        throw Services::ServiceRequestException("Insufficient permissions.");
      }
      auto accountPassword = std::string();
      try {
        accountPassword = m_dataStore->LoadPassword(entry);
      } catch(const ServiceLocatorDataStoreException&) {
        return;
      }
      if(!ValidatePassword(entry, password, accountPassword)) {
        return;
      }
      m_dataStore->StoreLastLoginTime(entry,
        boost::posix_time::second_clock::universal_time());
      account = entry;
    });
    return account;
  }

  template<typename C, typename D>
  DirectoryEntry ServiceLocatorServlet<C, D>::OnSessionAuthenticationRequest(
      ServiceProtocolClient& client, const std::string& sessionId,
      unsigned int saltId) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
    auto salt = std::to_string(saltId);
    auto upperCaseSessionId = boost::to_upper_copy(sessionId);
    return Threading::With(m_sessions, [&] (auto& sessions) {
      for(auto& session : sessions) {
        auto encodedSessionId = ComputeSHA(salt + session.first);
        if(encodedSessionId == upperCaseSessionId) {
          return session.second->GetSession().GetAccount();
        }
      }
      throw Services::ServiceRequestException("Session not found.");
    });
  }
}

#endif
