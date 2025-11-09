#ifndef BEAM_SERVICE_LOCATOR_SERVLET_HPP
#define BEAM_SERVICE_LOCATOR_SERVLET_HPP
#include <algorithm>
#include <atomic>
#include <random>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorSession.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Handles ServiceLocator service requests.
   * @tparam C The container instantiating this servlet.
   * @tparam D The type of data store to use.
   */
  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  class ServiceLocatorServlet {
    public:

      /** The type of ServiceLocatorDataStore used. */
      using ServiceLocatorDataStore = dereference_t<D>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a ServiceLocatorServlet.
       * @param data_store The data store to use.
       */
      template<Initializes<D> DF>
      explicit ServiceLocatorServlet(DF&& data_store);

      void register_services(Out<ServiceSlots<ServiceProtocolClient>> slots);
      void handle_close(ServiceProtocolClient& client);
      void close();

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
      local_ptr_t<D> m_data_store;
      SynchronizedVector<ServiceProtocolClient*, Mutex>
        m_account_update_subscribers;
      Sync<Sessions> m_sessions;
      Sync<ServiceListings> m_service_listings;
      Sync<ServiceEntryListings> m_service_entry_listings;
      Sync<DirectoryEntryMonitorEntries> m_directory_entry_monitor_entries;
      std::atomic_int m_next_service_id;
      OpenState m_open_state;

      ServiceLocatorServlet(const ServiceLocatorServlet&) = delete;
      ServiceLocatorServlet& operator =(const ServiceLocatorServlet&) = delete;
      void remove(const DirectoryEntry& entry);
      LoginServiceResult on_login_request(ServiceProtocolClient& client,
        const std::string& username, const std::string& password);
      ServiceEntry on_register_request(ServiceProtocolClient& client,
        const std::string& name, const JsonObject& properties);
      void on_unregister_request(ServiceProtocolClient& client,
        int service_id);
      std::vector<ServiceEntry> on_locate_request(ServiceProtocolClient& client,
        const std::string& name);
      std::vector<ServiceEntry> on_subscribe_request(
        ServiceProtocolClient& client, const std::string& service_name);
      void on_unsubscribe_request(ServiceProtocolClient& client,
        const std::string& service_name);
      std::vector<DirectoryEntry> on_monitor_directory_entry_request(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      DirectoryEntry on_load_path(ServiceProtocolClient& client,
        const DirectoryEntry& root, const std::string& path);
      std::vector<DirectoryEntry> on_monitor_accounts(
        ServiceProtocolClient& client);
      void on_unmonitor_accounts(ServiceProtocolClient& client);
      DirectoryEntry on_load_directory_entry(ServiceProtocolClient& client,
        unsigned int id);
      std::vector<DirectoryEntry> on_load_parents_request(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      std::vector<DirectoryEntry> on_load_children_request(
        ServiceProtocolClient& client, const DirectoryEntry& entry);
      std::vector<DirectoryEntry> on_load_all_accounts_request(
        ServiceProtocolClient& client);
      boost::optional<DirectoryEntry> on_find_account_request(
        ServiceProtocolClient& client, const std::string& name);
      DirectoryEntry on_make_account_request(ServiceProtocolClient& client,
        const std::string& name, const std::string& password,
        const DirectoryEntry& parent);
      DirectoryEntry on_make_directory_request(ServiceProtocolClient& client,
        const std::string& name, const DirectoryEntry& parent);
      void on_delete_directory_entry_request(ServiceProtocolClient& client,
        const DirectoryEntry& entry);
      void on_associate_request(ServiceProtocolClient& client,
        const DirectoryEntry& entry, const DirectoryEntry& parent);
      void on_detach_request(ServiceProtocolClient& client,
        const DirectoryEntry& entry, const DirectoryEntry& parent);
      void on_store_password_request(ServiceProtocolClient& client,
        const DirectoryEntry& account, const std::string& password);
      bool on_has_permissions_request(ServiceProtocolClient& client,
        const DirectoryEntry& account, const DirectoryEntry& target,
        Permissions permissions);
      void on_store_permissions_request(ServiceProtocolClient& client,
        const DirectoryEntry& source, const DirectoryEntry& target,
        Permissions permissions);
      boost::posix_time::ptime on_load_registration_time_request(
        ServiceProtocolClient& client, const DirectoryEntry& account);
      boost::posix_time::ptime on_load_last_login_time_request(
        ServiceProtocolClient& client, const DirectoryEntry& account);
      DirectoryEntry on_rename_request(ServiceProtocolClient& client,
        const DirectoryEntry& entry, const std::string& name);
      DirectoryEntry on_authenticate_account_request(
        ServiceProtocolClient& client, const std::string& username,
        const std::string& password);
      DirectoryEntry on_session_authentication_request(
        ServiceProtocolClient& client, const std::string& session_id,
        unsigned int salt_id);
  };

  template<typename D>
  struct MetaServiceLocatorServlet {
    using Session = ServiceLocatorSession;
    static constexpr auto SUPPORTS_PARALLELISM = true;

    template<typename C>
    struct apply {
      using type = ServiceLocatorServlet<C, D>;
    };
  };

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  template<Initializes<D> DF>
  ServiceLocatorServlet<C, D>::ServiceLocatorServlet(DF&& data_store)
      : m_data_store(std::forward<DF>(data_store)) {
    try {
      m_next_service_id = 1;
      m_data_store->with_transaction([&] {
        try {
          m_data_store->load_directory_entry(0);
          return;
        } catch(const ServiceLocatorDataStoreException&) {}
        auto star_directory = m_data_store->make_directory("*",
          DirectoryEntry::make_directory(static_cast<unsigned int>(-1), "*"));
        auto root_account = m_data_store->make_account("root", "",
          star_directory, boost::posix_time::second_clock::universal_time());
        m_data_store->set_permissions(
          root_account, star_directory, static_cast<Permissions>(~0));
      });
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::register_services(
      Out<ServiceSlots<ServiceProtocolClient>> slots) {
    ServiceLocatorServices::register_service_locator_services(out(slots));
    ServiceLocatorServices::register_service_locator_messages(out(slots));
    ServiceLocatorServices::LoginService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_login_request, this));
    ServiceLocatorServices::RegisterService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_register_request, this));
    ServiceLocatorServices::UnregisterService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_unregister_request, this));
    ServiceLocatorServices::LocateService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_locate_request, this));
    ServiceLocatorServices::SubscribeAvailabilityService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_subscribe_request, this));
    ServiceLocatorServices::UnsubscribeAvailabilityService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_unsubscribe_request, this));
    ServiceLocatorServices::MonitorDirectoryEntryService::add_slot(
      out(slots), std::bind_front(
        &ServiceLocatorServlet::on_monitor_directory_entry_request, this));
    ServiceLocatorServices::LoadPathService::add_slot(
      out(slots), std::bind_front(&ServiceLocatorServlet::on_load_path, this));
    ServiceLocatorServices::MonitorAccountsService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_monitor_accounts, this));
    ServiceLocatorServices::UnmonitorAccountsService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_unmonitor_accounts, this));
    ServiceLocatorServices::LoadDirectoryEntryService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_load_directory_entry, this));
    ServiceLocatorServices::LoadParentsService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_load_parents_request, this));
    ServiceLocatorServices::LoadChildrenService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_load_children_request, this));
    ServiceLocatorServices::LoadAllAccountsService::add_slot(
      out(slots), std::bind_front(
        &ServiceLocatorServlet::on_load_all_accounts_request, this));
    ServiceLocatorServices::FindAccountService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_find_account_request, this));
    ServiceLocatorServices::MakeAccountService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_make_account_request, this));
    ServiceLocatorServices::MakeDirectoryService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_make_directory_request, this));
    ServiceLocatorServices::DeleteDirectoryEntryService::add_slot(
      out(slots), std::bind_front(
        &ServiceLocatorServlet::on_delete_directory_entry_request, this));
    ServiceLocatorServices::AssociateService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_associate_request, this));
    ServiceLocatorServices::DetachService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_detach_request, this));
    ServiceLocatorServices::StorePasswordService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_store_password_request, this));
    ServiceLocatorServices::HasPermissionsService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_has_permissions_request, this));
    ServiceLocatorServices::StorePermissionsService::add_slot(
      out(slots), std::bind_front(
        &ServiceLocatorServlet::on_store_permissions_request, this));
    ServiceLocatorServices::LoadRegistrationTimeService::add_slot(
      out(slots),std::bind_front(
        &ServiceLocatorServlet::on_load_registration_time_request, this));
    ServiceLocatorServices::LoadLastLoginTimeService::add_slot(
      out(slots), std::bind_front(
        &ServiceLocatorServlet::on_load_last_login_time_request, this));
    ServiceLocatorServices::RenameService::add_slot(out(slots),
      std::bind_front(&ServiceLocatorServlet::on_rename_request, this));
    ServiceLocatorServices::AuthenticateAccountService::add_slot(
      out(slots), std::bind_front(
        &ServiceLocatorServlet::on_authenticate_account_request, this));
    ServiceLocatorServices::SessionAuthenticationService::add_slot(
      out(slots), std::bind_front(
        &ServiceLocatorServlet::on_session_authentication_request, this));
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::handle_close(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    auto registered_services = session.get_registered_services();
    auto service_subscriptions = session.get_service_subscriptions();
    with(m_service_entry_listings, [&] (auto& listings) {
      for(auto& service : registered_services) {
        auto& listing = listings[service.get_name()];
        std::erase(listing.m_entries, service);
        broadcast_record_message<
          ServiceLocatorServices::ServiceAvailabilityMessage>(
            listing.m_subscribers, service, false);
      }
      for(auto& subscription : service_subscriptions) {
        auto& listing = listings[subscription];
        std::erase(listing.m_subscribers, &client);
      }
    });
    auto monitors = session.get_monitors();
    with(m_directory_entry_monitor_entries, [&] (auto& entries) {
      for(auto& entry : monitors) {
        auto& monitor = entries[entry];
        std::erase(monitor.m_subscribers, &client);
      }
    });
    with(m_sessions, [&] (auto& sessions) {
      sessions.erase(session.get_session_id());
    });
    m_account_update_subscribers.erase(&client);
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_data_store->close();
    m_open_state.close();
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::remove(const DirectoryEntry& entry) {
    auto children = m_data_store->load_children(entry);
    for(auto& child : children) {
      auto parents = m_data_store->load_parents(child);
      if(parents.size() == 1) {
        remove(child);
      } else {
        m_data_store->detach(child, entry);
      }
    }
    auto parents = m_data_store->load_parents(entry);
    if(entry.m_type == DirectoryEntry::Type::ACCOUNT) {
      m_account_update_subscribers.for_each([&] (auto& subscriber) {
        if(has_permission(*m_data_store,
            subscriber->get_session().get_account(), entry, Permission::READ)) {
          send_record_message<ServiceLocatorServices::AccountUpdateMessage>(
            *subscriber, AccountUpdate::remove(entry));
        }
      });
    }
    m_data_store->remove(entry);
    with(m_directory_entry_monitor_entries, [&] (auto& entries) {
      auto monitor_iterator = entries.find(entry);
      if(monitor_iterator == entries.end()) {
        return;
      }
      auto& monitor = monitor_iterator->second;
      for(auto& parent : parents) {
        broadcast_record_message<
          ServiceLocatorServices::DirectoryEntryDetachedMessage>(
            monitor.m_subscribers, entry, parent);
      }
      entries.erase(monitor_iterator);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  LoginServiceResult ServiceLocatorServlet<C, D>::on_login_request(
      ServiceProtocolClient& client, const std::string& username,
      const std::string& password) {
    auto& session = client.get_session();
    if(!session.try_login()) {
      boost::throw_with_location(
        ServiceRequestException("Account is already logged in."));
    }
    auto account = DirectoryEntry();
    try {
      m_data_store->with_transaction([&] {
        try {
          account = m_data_store->load_account(username);
        } catch(const ServiceLocatorDataStoreException&) {
          boost::throw_with_location(
            ServiceRequestException("Invalid username or password."));
        }
        auto account_password = std::string();
        try {
          account_password = m_data_store->load_password(account);
        } catch(const ServiceLocatorDataStoreException&) {
          boost::throw_with_location(ServiceRequestException(
            "Unable to retrieve password, try again later."));
        }
        if(!validate_password(account, password, account_password)) {
          boost::throw_with_location(
            ServiceRequestException("Invalid username or password."));
        }
        m_data_store->store_last_login_time(
          account, boost::posix_time::second_clock::universal_time());
      });
    } catch(const std::exception&) {
      session.reset_login();
      throw;
    }
    auto session_id = std::string();
    with(m_sessions, [&] (auto& sessions) {
      do {
        session_id = generate_session_id();
      } while(sessions.find(session_id) != sessions.end());
      sessions.insert(std::make_pair(session_id, &client));
    });
    session.set_session_id(account, session_id);
    return LoginServiceResult(account, session_id);
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  ServiceEntry ServiceLocatorServlet<C, D>::on_register_request(
      ServiceProtocolClient& client, const std::string& name,
      const JsonObject& properties) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    auto id = ++m_next_service_id;
    auto entry = ServiceEntry(name, properties, id, session.get_account());
    with(m_service_entry_listings, m_service_listings,
      [&] (auto& listings, auto& service_listings) {
        auto& listing = listings[name];
        listing.m_entries.push_back(entry);
        service_listings.insert(std::make_pair(id, entry));
        session.register_service(entry);
        broadcast_record_message<
          ServiceLocatorServices::ServiceAvailabilityMessage>(
            listing.m_subscribers, entry, true);
      });
    return entry;
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_unregister_request(
      ServiceProtocolClient& client, int service_id) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    auto registered_services = session.get_registered_services();
    auto service_found = false;
    for(auto& service : registered_services) {
      if(service.get_id() == service_id) {
        service_found = true;
        break;
      }
    }
    if(!service_found) {
      boost::throw_with_location(ServiceRequestException("Service not found."));
    }
    with(m_service_entry_listings, m_service_listings,
      [&] (auto& listings, auto& service_listings) {
        auto& entry = service_listings[service_id];
        auto& listing = listings[entry.get_name()];
        std::erase(listing.m_entries, entry);
        session.unregister_service(service_id);
        broadcast_record_message<
          ServiceLocatorServices::ServiceAvailabilityMessage>(
            listing.m_subscribers, entry, false);
      });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<ServiceEntry> ServiceLocatorServlet<C, D>::on_locate_request(
      ServiceProtocolClient& client, const std::string& name) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    auto listings = with(m_service_entry_listings,
      [&] (auto& listings) -> std::vector<ServiceEntry> {
        auto entry_iterator = listings.find(name);
        if(entry_iterator != listings.end()) {
          return entry_iterator->second.m_entries;
        }
        return {};
      });
    auto random_device = std::random_device();
    auto random_generator = std::mt19937(random_device());
    std::shuffle(listings.begin(), listings.end(), random_generator);
    return listings;
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<ServiceEntry> ServiceLocatorServlet<C, D>::on_subscribe_request(
      ServiceProtocolClient& client, const std::string& service_name) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return with(m_service_entry_listings,
      [&] (auto& listings) -> std::vector<ServiceEntry> {
        auto& listing = listings[service_name];
        if(std::ranges::contains(
            session.get_service_subscriptions(), service_name)) {
          return {};
        }
        listing.m_subscribers.push_back(&client);
        session.subscribe_service(service_name);
        return listing.m_entries;
      });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_unsubscribe_request(
      ServiceProtocolClient& client, const std::string& service_name) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    with(m_service_entry_listings, [&] (auto& listings) {
      auto& listing = listings[service_name];
      std::erase(listing.m_subscribers, &client);
      session.unsubscribe_service(service_name);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::
      on_monitor_directory_entry_request(ServiceProtocolClient& client,
        const DirectoryEntry& entry) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    auto parents = m_data_store->with_transaction([&] {
      auto validated_entry = validate(*m_data_store, entry);
      if(!has_permission(*m_data_store, session.get_account(), validated_entry,
          Permission::READ)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return m_data_store->load_parents(validated_entry);
    });
    with(m_directory_entry_monitor_entries, [&] (auto& entries) {
      auto& monitor = entries[entry];
      if(std::ranges::contains(monitor.m_subscribers, &client)) {
        boost::throw_with_location(
          ServiceRequestException("Already subscribed."));
      }
      monitor.m_subscribers.push_back(&client);
      session.monitor(entry);
    });
    return parents;
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry ServiceLocatorServlet<C, D>::on_load_path(
      ServiceProtocolClient& client, const DirectoryEntry& root,
      const std::string& path) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto entry = load_directory_entry(*m_data_store, root, path);
      if(!has_permission(
          *m_data_store, session.get_account(), entry, Permission::READ)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return entry;
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::on_monitor_accounts(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      return m_account_update_subscribers.with([&] (auto& subscribers) {
        if(!std::ranges::contains(subscribers, &client)) {
          subscribers.push_back(&client);
        }
        auto accounts = m_data_store->load_all_accounts();
        std::erase_if(accounts, [&] (auto& account) {
          return !has_permission(
            *m_data_store, session.get_account(), account, Permission::READ);
        });
        return accounts;
      });
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_unmonitor_accounts(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    m_account_update_subscribers.erase(&client);
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry ServiceLocatorServlet<C, D>::on_load_directory_entry(
      ServiceProtocolClient& client, unsigned int id) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto entry = m_data_store->load_directory_entry(id);
      if(!has_permission(
          *m_data_store, session.get_account(), entry, Permission::READ)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return entry;
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::
      on_load_parents_request(
        ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_entry = validate(*m_data_store, entry);
      if(!has_permission(*m_data_store, session.get_account(), validated_entry,
          Permission::READ)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return m_data_store->load_parents(validated_entry);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::
      on_load_children_request(
        ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_entry = validate(*m_data_store, entry);
      if(!has_permission(*m_data_store, session.get_account(), validated_entry,
          Permission::READ)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return m_data_store->load_children(validated_entry);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  std::vector<DirectoryEntry> ServiceLocatorServlet<C, D>::
      on_load_all_accounts_request(ServiceProtocolClient& client) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto accounts = m_data_store->load_all_accounts();
      std::erase_if(accounts, [&] (auto& account) {
        return !has_permission(
          *m_data_store, session.get_account(), account, Permission::READ);
      });
      return accounts;
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  boost::optional<DirectoryEntry> ServiceLocatorServlet<C, D>::
      on_find_account_request(
        ServiceProtocolClient& client, const std::string& name) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction(
      [&] () -> boost::optional<DirectoryEntry> {
        try {
          return m_data_store->load_account(name);
        } catch(const ServiceLocatorDataStoreException&) {}
        return boost::none;
      });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry ServiceLocatorServlet<C, D>::on_make_account_request(
      ServiceProtocolClient& client, const std::string& name,
      const std::string& password, const DirectoryEntry& parent) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    auto validated_name = boost::trim_copy(name);
    if(validated_name.empty()) {
      boost::throw_with_location(ServiceRequestException("Name is empty."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_parent = validate(*m_data_store, parent);
      if(!has_permission(*m_data_store, session.get_account(), validated_parent,
          Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      auto new_entry = m_data_store->make_account(validated_name, password,
        validated_parent, boost::posix_time::second_clock::universal_time());
      m_account_update_subscribers.for_each([&] (auto& subscriber) {
        if(has_permission(
            *m_data_store, subscriber->get_session().get_account(), new_entry,
            Permission::READ)) {
          send_record_message<ServiceLocatorServices::AccountUpdateMessage>(
            *subscriber, AccountUpdate::add(new_entry));
        }
      });
      return new_entry;
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry ServiceLocatorServlet<C, D>::on_make_directory_request(
      ServiceProtocolClient& client, const std::string& name,
      const DirectoryEntry& parent) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    auto validated_name = boost::trim_copy(name);
    if(validated_name.empty()) {
      boost::throw_with_location(ServiceRequestException("Name is empty."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_parent = validate(*m_data_store, parent);
      if(!has_permission(*m_data_store, session.get_account(), validated_parent,
          Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return m_data_store->make_directory(validated_name, validated_parent);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_delete_directory_entry_request(
      ServiceProtocolClient& client, const DirectoryEntry& entry) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    m_data_store->with_transaction([&] {
      auto validated_entry = validate(*m_data_store, entry);
      if(!has_permission(*m_data_store, session.get_account(), validated_entry,
          Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      remove(validated_entry);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_associate_request(
      ServiceProtocolClient& client, const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    m_data_store->with_transaction([&] {
      auto validated_entry = validate(*m_data_store, entry);
      auto validated_parent = validate(*m_data_store, parent);
      if(!has_permission(*m_data_store, session.get_account(), validated_parent,
          Permission::MOVE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      if(!m_data_store->associate(validated_entry, validated_parent)) {
        return;
      }
      with(m_directory_entry_monitor_entries, [&] (auto& entries) {
        auto monitor_iterator = entries.find(validated_entry);
        if(monitor_iterator == entries.end()) {
          return;
        }
        auto& monitor = monitor_iterator->second;
        broadcast_record_message<
          ServiceLocatorServices::DirectoryEntryAssociatedMessage>(
            monitor.m_subscribers, validated_entry, validated_parent);
      });
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_detach_request(
      ServiceProtocolClient& client, const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    m_data_store->with_transaction([&] {
      auto validated_entry = validate(*m_data_store, entry);
      auto validated_parent = validate(*m_data_store, parent);
      if(!has_permission(*m_data_store, session.get_account(), validated_parent,
          Permission::MOVE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      auto parents = m_data_store->load_parents(validated_entry);
      if(parents.size() == 1) {
        boost::throw_with_location(ServiceRequestException(
          "Entry only has one parent, must be deleted instead of detached."));
      }
      if(!m_data_store->detach(validated_entry, validated_parent)) {
        return;
      }
      with(m_directory_entry_monitor_entries, [&] (auto& entries) {
        auto monitor_iterator = entries.find(validated_entry);
        if(monitor_iterator == entries.end()) {
          return;
        }
        auto& monitor = monitor_iterator->second;
        broadcast_record_message<
          ServiceLocatorServices::DirectoryEntryDetachedMessage>(
            monitor.m_subscribers, validated_entry, validated_parent);
      });
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_store_password_request(
      ServiceProtocolClient& client, const DirectoryEntry& account,
      const std::string& password) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    m_data_store->with_transaction([&] {
      auto validated_account = validate(*m_data_store, account);
      if(validated_account != session.get_account() &&
          !has_permission(*m_data_store, session.get_account(),
            validated_account, Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      m_data_store->set_password(
        validated_account, hash_password(validated_account, password));
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  bool ServiceLocatorServlet<C, D>::on_has_permissions_request(
      ServiceProtocolClient& client, const DirectoryEntry& account,
      const DirectoryEntry& target, Permissions permissions) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_account = validate(*m_data_store, account);
      auto validated_target = validate(*m_data_store, target);
      if(!has_permission(*m_data_store, session.get_account(),
          validated_account, Permission::ADMINISTRATE) ||
          !has_permission(*m_data_store, session.get_account(),
            validated_target, Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return has_permission(*m_data_store, account, target, permissions);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  void ServiceLocatorServlet<C, D>::on_store_permissions_request(
      ServiceProtocolClient& client, const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    m_data_store->with_transaction([&] {
      auto validated_source = validate(*m_data_store, source);
      auto validated_target = validate(*m_data_store, target);
      if(!has_permission(*m_data_store, session.get_account(), validated_source,
          Permission::ADMINISTRATE) || !has_permission(
            *m_data_store, session.get_account(), validated_target,
            Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      m_data_store->set_permissions(source, validated_target, permissions);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  boost::posix_time::ptime ServiceLocatorServlet<C, D>::
      on_load_registration_time_request(
        ServiceProtocolClient& client, const DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_account = validate(*m_data_store, account);
      if(!has_permission(*m_data_store, session.get_account(),
          validated_account, Permission::READ)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return m_data_store->load_registration_time(validated_account);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  boost::posix_time::ptime ServiceLocatorServlet<C, D>::
      on_load_last_login_time_request(
        ServiceProtocolClient& client, const DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_account = validate(*m_data_store, account);
      if(!has_permission(*m_data_store, session.get_account(),
          validated_account, Permission::READ)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      return m_data_store->load_last_login_time(validated_account);
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry ServiceLocatorServlet<C, D>::on_rename_request(
      ServiceProtocolClient& client, const DirectoryEntry& entry,
      const std::string& name) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto validated_entry = validate(*m_data_store, entry);
      if(!has_permission(*m_data_store, session.get_account(), validated_entry,
          Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      auto is_existing_account = [&] {
        try {
          m_data_store->load_account(name);
          return true;
        } catch(const ServiceLocatorDataStoreException&) {
          return false;
        }
      }();
      if(is_existing_account) {
        boost::throw_with_location(ServiceRequestException(
          "An account with the specified name exists."));
      }
      m_data_store->rename(validated_entry, name);
      validated_entry.m_name = name;
      return validated_entry;
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry ServiceLocatorServlet<C, D>::on_authenticate_account_request(
      ServiceProtocolClient& client, const std::string& username,
      const std::string& password) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    return m_data_store->with_transaction([&] {
      auto entry = DirectoryEntry();
      try {
        entry = m_data_store->load_account(username);
      } catch(const ServiceLocatorDataStoreException&) {
        return DirectoryEntry();
      }
      if(!has_permission(*m_data_store, session.get_account(), entry,
          Permission::ADMINISTRATE)) {
        boost::throw_with_location(
          ServiceRequestException("Insufficient permissions."));
      }
      auto account_password = std::string();
      try {
        account_password = m_data_store->load_password(entry);
      } catch(const ServiceLocatorDataStoreException&) {
        return DirectoryEntry();
      }
      if(!validate_password(entry, password, account_password)) {
        return DirectoryEntry();
      }
      m_data_store->store_last_login_time(
        entry, boost::posix_time::second_clock::universal_time());
      return entry;
    });
  }

  template<typename C, typename D> requires
    IsServiceLocatorDataStore<dereference_t<D>>
  DirectoryEntry ServiceLocatorServlet<C, D>::on_session_authentication_request(
      ServiceProtocolClient& client, const std::string& session_id,
      unsigned int salt_id) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
    auto salt = std::to_string(salt_id);
    auto upper_case_session_id = boost::to_upper_copy(session_id);
    return with(m_sessions, [&] (auto& sessions) {
      for(auto& session : sessions) {
        auto encoded_session_id = compute_sha(salt + session.first);
        if(encoded_session_id == upper_case_session_id) {
          return session.second->get_session().get_account();
        }
      }
      boost::throw_with_location(ServiceRequestException("Session not found."));
    });
  }
}

#endif
