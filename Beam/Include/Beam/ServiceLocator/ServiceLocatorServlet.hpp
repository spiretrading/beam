#ifndef BEAM_SERVICELOCATORSERVLET_HPP
#define BEAM_SERVICELOCATORSERVLET_HPP
#include <algorithm>
#include <random>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/ServiceLocatorSession.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class ServiceLocatorServlet
      \brief Handles ServiceLocator service requests.
      \tparam ContainerType The container instantiating this servlet.
      \tparam ServiceLocatorDataStoreType The type of data store to use.
   */
  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  class ServiceLocatorServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of ServiceLocatorDataStore used.
      using ServiceLocatorDataStore =
        GetTryDereferenceType<ServiceLocatorDataStoreType>;

      //! Constructs a ServiceLocatorServlet.
      /*!
        \param dataStore The data store to use.
      */
      template<typename DataStoreForward>
      ServiceLocatorServlet(DataStoreForward&& dataStore);

      void RegisterServices(
        Out<Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

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
      GetOptionalLocalPtr<ServiceLocatorDataStoreType> m_dataStore;
      Threading::Sync<Sessions> m_sessions;
      Threading::Sync<ServiceListings> m_serviceListings;
      Threading::Sync<ServiceEntryListings> m_serviceEntryListings;
      Threading::Sync<DirectoryEntryMonitorEntries>
        m_directoryEntryMonitorEntries;
      boost::atomic<int> m_nextServiceId;
      IO::OpenState m_openState;

      void Shutdown();
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
      DirectoryEntry OnLoadDirectoryEntry(ServiceProtocolClient& client,
        unsigned int id);
      std::vector<DirectoryEntry> OnLoadParentsRequest(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      std::vector<DirectoryEntry> OnLoadChildrenRequest(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      std::vector<DirectoryEntry> OnLoadAllAccountsRequest(
        ServiceProtocolClient& client, int dummy);
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

  template<typename ServiceLocatorDataStoreType>
  struct MetaServiceLocatorServlet {
    static constexpr bool SupportsParallelism = true;
    using Session = ServiceLocatorSession;
    template<typename ContainerType>
    struct apply {
      using type =
        ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>;
    };
  };

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  template<typename DataStoreForward>
  ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      ServiceLocatorServlet(DataStoreForward&& dataStore)
      : m_dataStore{std::forward<DataStoreForward>(dataStore)} {}

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      RegisterServices(Out<Services::ServiceSlots<ServiceProtocolClient>>
      slots) {
    RegisterServiceLocatorServices(Store(slots));
    RegisterServiceLocatorMessages(Store(slots));
    LoginService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoginRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    RegisterService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnRegisterRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    UnregisterService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnUnregisterRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    LocateService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLocateRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    SubscribeAvailabilityService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnSubscribeRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    UnsubscribeAvailabilityService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnUnsubscribeRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    MonitorDirectoryEntryService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnMonitorDirectoryEntryRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadPathService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoadPath, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    LoadDirectoryEntryService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoadDirectoryEntry, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadParentsService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoadParentsRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadChildrenService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoadChildrenRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAllAccountsService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoadAllAccountsRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    FindAccountService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnFindAccountRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    MakeAccountService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnMakeAccountRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    MakeDirectoryService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnMakeDirectoryRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    DeleteDirectoryEntryService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnDeleteDirectoryEntryRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    AssociateService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnAssociateRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    DetachService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnDetachRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    StorePasswordService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnStorePasswordRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    HasPermissionsService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnHasPermissionsRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4));
    StorePermissionsService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnStorePermissionsRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4));
    LoadRegistrationTimeService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoadRegistrationTimeRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadLastLoginTimeService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnLoadLastLoginTimeRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    RenameService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnRenameRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    AuthenticateAccountService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnAuthenticateAccountRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    SessionAuthenticationService::AddSlot(Store(slots), std::bind(
      &ServiceLocatorServlet::OnSessionAuthenticationRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      HandleClientClosed(ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    auto registeredServices = session.GetRegisteredServices();
    auto serviceSubscriptions = session.GetServiceSubscriptions();
    Threading::With(m_serviceEntryListings,
      [&] (ServiceEntryListings& serviceEntryListings) {
        for(auto& registeredService : registeredServices) {
          auto& listing = serviceEntryListings[registeredService.GetName()];
          RemoveAll(listing.m_entries, registeredService);
          for(auto subscriber : listing.m_subscribers) {
            Services::SendRecordMessage<ServiceAvailabilityMessage>(*subscriber,
              registeredService, false);
          }
        }
        for(auto& serviceSubscription : serviceSubscriptions) {
          auto& listing = serviceEntryListings[serviceSubscription];
          RemoveAll(listing.m_subscribers, &client);
        }
      });
    auto monitors = session.GetMonitors();
    Threading::With(m_directoryEntryMonitorEntries,
      [&] (DirectoryEntryMonitorEntries& directoryEntryMonitorEntries) {
        for(auto& entry : monitors) {
          auto& monitor = directoryEntryMonitorEntries[entry];
          RemoveAll(monitor.m_subscribers, &client);
        }
      });
    Threading::With(m_sessions,
      [&] (Sessions& sessions) {
        sessions.erase(session.GetSessionId());
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_nextServiceId = 1;
      m_dataStore->Open();
      m_dataStore->WithTransaction(
        [&] {
          try {
            m_dataStore->LoadDirectoryEntry(0);
            return;
          } catch(const ServiceLocatorDataStoreException&) {}
          auto starDirectory = m_dataStore->MakeDirectory("*",
            DirectoryEntry{DirectoryEntry::Type::DIRECTORY,
            static_cast<unsigned int>(-1), "*"});
          auto rootAccount = m_dataStore->MakeAccount("root", "", starDirectory,
            boost::posix_time::second_clock::universal_time());
          m_dataStore->SetPermissions(rootAccount, starDirectory,
            static_cast<Permissions>(~0));
        });
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      Shutdown() {
    m_dataStore->Close();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      Delete(const DirectoryEntry& entry) {
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
    m_dataStore->Delete(entry);
    Threading::With(m_directoryEntryMonitorEntries,
      [&] (DirectoryEntryMonitorEntries& directoryEntryMonitorEntries) {
        auto monitorIterator = directoryEntryMonitorEntries.find(entry);
        if(monitorIterator == directoryEntryMonitorEntries.end()) {
          return;
        }
        auto& monitor = monitorIterator->second;
        for(auto& parent : parents) {
          for(auto subscriber : monitor.m_subscribers) {
            Services::SendRecordMessage<DirectoryEntryDetachedMessage>(
              *subscriber, entry, parent);
          }
        }
        directoryEntryMonitorEntries.erase(monitorIterator);
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  LoginServiceResult ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoginRequest(
      ServiceProtocolClient& client, const std::string& username,
      const std::string& password) {
    auto& session = client.GetSession();
    if(!session.TryLogin()) {
      throw Services::ServiceRequestException{"Account is already logged in."};
    }
    DirectoryEntry account;
    try {
      m_dataStore->WithTransaction(
        [&] {
          try {
            account = m_dataStore->LoadAccount(username);
          } catch(const ServiceLocatorDataStoreException&) {
            throw Services::ServiceRequestException{
              "Invalid username or password."};
          }
          std::string accountPassword;
          try {
            accountPassword = m_dataStore->LoadPassword(account);
          } catch(const ServiceLocatorDataStoreException&) {
            throw Services::ServiceRequestException{
              "Unable to retrieve password, try again later."};
          }
          if(!ValidatePassword(account, password, accountPassword)) {
            throw Services::ServiceRequestException{
              "Invalid username or password."};
          }
          m_dataStore->StoreLastLoginTime(account,
            boost::posix_time::second_clock::universal_time());
        });
    } catch(const std::exception&) {
      session.ResetLogin();
      throw;
    }
    std::string sessionId;
    Threading::With(m_sessions,
      [&] (Sessions& sessions) {
        do {
          sessionId = GenerateSessionId();
        } while(sessions.find(sessionId) != sessions.end());
        sessions.insert(std::make_pair(sessionId, &client));
      });
    session.SetSessionId(account, sessionId);
    return LoginServiceResult{account, sessionId};
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  ServiceEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnRegisterRequest(
      ServiceProtocolClient& client, const std::string& name,
      const JsonObject& properties) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    auto id = ++m_nextServiceId;
    ServiceEntry entry{name, properties, id, session.GetAccount()};
    Threading::With(m_serviceEntryListings, m_serviceListings,
      [&] (ServiceEntryListings& serviceEntryListings,
          ServiceListings& serviceListings) {
        auto& listing = serviceEntryListings[name];
        listing.m_entries.push_back(entry);
        serviceListings.insert(std::make_pair(id, entry));
        session.RegisterService(entry);
        for(auto subscriber : listing.m_subscribers) {
          Services::SendRecordMessage<ServiceAvailabilityMessage>(*subscriber,
            entry, true);
        }
      });
    return entry;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnUnregisterRequest(ServiceProtocolClient& client, int serviceId) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
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
      throw Services::ServiceRequestException{"Service not found."};
    }
    Threading::With(m_serviceEntryListings, m_serviceListings,
      [&] (ServiceEntryListings& serviceEntryListings,
          ServiceListings& serviceListings) {
        auto& entry = serviceListings[serviceId];
        auto& listing = serviceEntryListings[entry.GetName()];
        RemoveAll(listing.m_entries, entry);
        session.UnregisterService(serviceId);
        for(auto subscriber : listing.m_subscribers) {
          Services::SendRecordMessage<ServiceAvailabilityMessage>(*subscriber,
            entry, false);
        }
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  std::vector<ServiceEntry> ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLocateRequest(
      ServiceProtocolClient& client, const std::string& name) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    auto listings = Threading::With(m_serviceEntryListings,
      [&] (ServiceEntryListings& serviceEntryListings) ->
          std::vector<ServiceEntry> {
        auto entryIterator = serviceEntryListings.find(name);
        if(entryIterator != serviceEntryListings.end()) {
          return entryIterator->second.m_entries;
        }
        return {};
      });
    std::random_device randomDevice;
    std::mt19937 randomGenerator{randomDevice()};
    std::shuffle(listings.begin(), listings.end(), randomGenerator);
    return listings;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  std::vector<ServiceEntry> ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnSubscribeRequest(
      ServiceProtocolClient& client, const std::string& serviceName) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    return Threading::With(m_serviceEntryListings,
      [&] (ServiceEntryListings& serviceEntryListings) ->
          std::vector<ServiceEntry> {
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

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnUnsubscribeRequest(ServiceProtocolClient& client,
      const std::string& serviceName) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    Threading::With(m_serviceEntryListings,
      [&] (ServiceEntryListings& serviceEntryListings) {
        auto& listing = serviceEntryListings[serviceName];
        RemoveAll(listing.m_subscribers, &client);
        session.UnsubscribeService(serviceName);
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  std::vector<DirectoryEntry> ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnMonitorDirectoryEntryRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    std::vector<DirectoryEntry> parents;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(entry);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
            Permission::READ)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        parents = m_dataStore->LoadParents(validatedEntry);
      });
    Threading::With(m_directoryEntryMonitorEntries,
      [&] (DirectoryEntryMonitorEntries& directoryEntryMonitorEntries) {
        auto& monitor = directoryEntryMonitorEntries[entry];
        if(std::find(monitor.m_subscribers.begin(), monitor.m_subscribers.end(),
            &client) != monitor.m_subscribers.end()) {
          throw Services::ServiceRequestException{"Already subscribed."};
        }
        monitor.m_subscribers.push_back(&client);
        session.Monitor(entry);
      });
    return parents;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  DirectoryEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoadPath(ServiceProtocolClient& client,
      const DirectoryEntry& root, const std::string& path) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    DirectoryEntry entry;
    m_dataStore->WithTransaction(
      [&] {
        entry = LoadDirectoryEntry(*m_dataStore, root, path);
        if(!HasPermission(*m_dataStore, session.GetAccount(), entry,
            Permission::READ)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
      });
    return entry;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  DirectoryEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoadDirectoryEntry(
      ServiceProtocolClient& client, unsigned int id) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    DirectoryEntry entry;
    m_dataStore->WithTransaction(
      [&] {
        entry = m_dataStore->LoadDirectoryEntry(id);
        if(!HasPermission(*m_dataStore, session.GetAccount(), entry,
            Permission::READ)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
      });
    return entry;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  std::vector<DirectoryEntry> ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoadParentsRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    std::vector<DirectoryEntry> parents;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(entry);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
            Permission::READ)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        parents = m_dataStore->LoadParents(validatedEntry);
      });
    return parents;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  std::vector<DirectoryEntry> ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoadChildrenRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    std::vector<DirectoryEntry> children;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(entry);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
            Permission::READ)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        children = m_dataStore->LoadChildren(validatedEntry);
      });
    return children;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  std::vector<DirectoryEntry> ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoadAllAccountsRequest(
      ServiceProtocolClient& client, int dummy) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    std::vector<DirectoryEntry> accounts;
    m_dataStore->WithTransaction(
      [&] {
        try {
          accounts = m_dataStore->LoadAllAccounts();
          auto i = accounts.begin();
          while(i != accounts.end()) {
            if(!HasPermission(*m_dataStore, session.GetAccount(), *i,
                Permission::READ)) {
              i = accounts.erase(i);
            } else {
              ++i;
            }
          }
        } catch(const ServiceLocatorDataStoreException&) {
        }
      });
    return accounts;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  boost::optional<DirectoryEntry> ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnFindAccountRequest(
      ServiceProtocolClient& client, const std::string& name) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    boost::optional<DirectoryEntry> account;
    m_dataStore->WithTransaction(
      [&] {
        try {
          account = m_dataStore->LoadAccount(name);
        } catch(const ServiceLocatorDataStoreException&) {}
      });
    return account;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  DirectoryEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnMakeAccountRequest(
      ServiceProtocolClient& client, const std::string& name,
      const std::string& password, const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    auto validatedName = boost::trim_copy(name);
    if(validatedName.empty()) {
      throw Services::ServiceRequestException{"Name is empty."};
    }
    DirectoryEntry newEntry;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedParent = m_dataStore->Validate(parent);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
            Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        newEntry = m_dataStore->MakeAccount(validatedName, password,
          validatedParent, boost::posix_time::second_clock::universal_time());
      });
    return newEntry;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  DirectoryEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnMakeDirectoryRequest(
      ServiceProtocolClient& client, const std::string& name,
      const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    auto validatedName = boost::trim_copy(name);
    if(validatedName.empty()) {
      throw Services::ServiceRequestException{"Name is empty."};
    }
    DirectoryEntry newEntry;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedParent = m_dataStore->Validate(parent);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
            Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        newEntry = m_dataStore->MakeDirectory(validatedName, validatedParent);
      });
    return newEntry;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnDeleteDirectoryEntryRequest(ServiceProtocolClient& client,
      const DirectoryEntry& entry) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(entry);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
            Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        Delete(validatedEntry);
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnAssociateRequest(ServiceProtocolClient& client,
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(entry);
        auto validatedParent = m_dataStore->Validate(parent);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
            Permission::MOVE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        if(!m_dataStore->Associate(validatedEntry, validatedParent)) {
          return;
        }
        Threading::With(m_directoryEntryMonitorEntries,
          [&] (DirectoryEntryMonitorEntries& directoryEntryMonitorEntries) {
            auto monitorIterator = directoryEntryMonitorEntries.find(
              validatedEntry);
            if(monitorIterator == directoryEntryMonitorEntries.end()) {
              return;
            }
            auto& monitor = monitorIterator->second;
            for(auto subscriber : monitor.m_subscribers) {
              Services::SendRecordMessage<DirectoryEntryAssociatedMessage>(
                *subscriber, validatedEntry, validatedParent);
            }
          });
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnDetachRequest(ServiceProtocolClient& client,
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(entry);
        auto validatedParent = m_dataStore->Validate(parent);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedParent,
            Permission::MOVE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        auto parents = m_dataStore->LoadParents(validatedEntry);
        if(parents.size() == 1) {
          throw Services::ServiceRequestException{
            "Entry only has one parent, must be deleted instead of detached."};
        }
        if(!m_dataStore->Detach(validatedEntry, validatedParent)) {
          return;
        }
        Threading::With(m_directoryEntryMonitorEntries,
          [&] (DirectoryEntryMonitorEntries& directoryEntryMonitorEntries) {
            auto monitorIterator = directoryEntryMonitorEntries.find(
              validatedEntry);
            if(monitorIterator == directoryEntryMonitorEntries.end()) {
              return;
            }
            auto& monitor = monitorIterator->second;
            for(auto subscriber : monitor.m_subscribers) {
              Services::SendRecordMessage<DirectoryEntryDetachedMessage>(
                *subscriber, validatedEntry, validatedParent);
            }
          });
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnStorePasswordRequest(ServiceProtocolClient& client,
      const DirectoryEntry& account, const std::string& password) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    m_dataStore->WithTransaction(
      [&] {
        auto validatedAccount = m_dataStore->Validate(account);
        if(validatedAccount != session.GetAccount() &&
            !HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
            Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        m_dataStore->SetPassword(validatedAccount,
          HashPassword(validatedAccount, password));
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  bool ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnHasPermissionsRequest(ServiceProtocolClient& client,
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    auto result = false;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedAccount = m_dataStore->Validate(account);
        auto validatedTarget = m_dataStore->Validate(target);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
            Permission::ADMINISTRATE) || !HasPermission(*m_dataStore,
            session.GetAccount(), validatedTarget, Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        result = HasPermission(*m_dataStore, account, target, permissions);
      });
    return result;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  void ServiceLocatorServlet<ContainerType, ServiceLocatorDataStoreType>::
      OnStorePermissionsRequest(ServiceProtocolClient& client,
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    m_dataStore->WithTransaction(
      [&] {
        auto validatedSource = m_dataStore->Validate(source);
        auto validatedTarget = m_dataStore->Validate(target);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedSource,
            Permission::ADMINISTRATE) ||
            !HasPermission(*m_dataStore, session.GetAccount(), validatedTarget,
            Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        m_dataStore->SetPermissions(source, validatedTarget, permissions);
      });
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  boost::posix_time::ptime ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoadRegistrationTimeRequest(
      ServiceProtocolClient& client, const DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    boost::posix_time::ptime registrationTime;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedAccount = m_dataStore->Validate(account);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
            Permission::READ)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        registrationTime = m_dataStore->LoadRegistrationTime(validatedAccount);
      });
    return registrationTime;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  boost::posix_time::ptime ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnLoadLastLoginTimeRequest(
      ServiceProtocolClient& client, const DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    boost::posix_time::ptime lastLoginTime;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedAccount = m_dataStore->Validate(account);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedAccount,
            Permission::READ)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        lastLoginTime = m_dataStore->LoadLastLoginTime(validatedAccount);
      });
    return lastLoginTime;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  DirectoryEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnRenameRequest(
      ServiceProtocolClient& client, const DirectoryEntry& entry,
      const std::string& name) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    DirectoryEntry result;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(entry);
        if(!HasPermission(*m_dataStore, session.GetAccount(), validatedEntry,
            Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        auto isExistingAccount = false;
        try {
          m_dataStore->LoadAccount(name);
          isExistingAccount = true;
        } catch(const ServiceLocatorDataStoreException&) {
          isExistingAccount = false;
        }
        if(isExistingAccount) {
          throw Services::ServiceRequestException{
            "An account with the specified name exists."};
        }
        m_dataStore->Rename(validatedEntry, name);
        result = validatedEntry;
        result.m_name = name;
      });
    return result;
  }

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  DirectoryEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnAuthenticateAccountRequest(
      ServiceProtocolClient& client, const std::string& username,
      const std::string& password) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    DirectoryEntry account;
    m_dataStore->WithTransaction(
      [&] {
        DirectoryEntry entry;
        try {
          entry = m_dataStore->LoadAccount(username);
        } catch(const ServiceLocatorDataStoreException&) {
          return;
        }
        if(!HasPermission(*m_dataStore, session.GetAccount(), entry,
            Permission::ADMINISTRATE)) {
          throw Services::ServiceRequestException{"Insufficient permissions."};
        }
        std::string accountPassword;
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

  template<typename ContainerType, typename ServiceLocatorDataStoreType>
  DirectoryEntry ServiceLocatorServlet<ContainerType,
      ServiceLocatorDataStoreType>::OnSessionAuthenticationRequest(
      ServiceProtocolClient& client, const std::string& sessionId,
      unsigned int saltId) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
    auto salt = ToString(saltId);
    auto upperCaseSessionId = boost::to_upper_copy(sessionId);
    return Threading::With(m_sessions,
      [&] (Sessions& sessions) -> DirectoryEntry {
        for(auto& session : sessions) {
          auto encodedSessionId = ComputeSHA(salt + session.first);
          if(encodedSessionId == upperCaseSessionId) {
            return session.second->GetSession().GetAccount();
          }
        }
        throw Services::ServiceRequestException{"Session not found."};
      });
  }
}
}

#endif
