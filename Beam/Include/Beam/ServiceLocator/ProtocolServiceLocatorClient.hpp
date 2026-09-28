#ifndef BEAM_PROTOCOL_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_PROTOCOL_SERVICE_LOCATOR_CLIENT_HPP
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Implements a ServiceLocatorClient using Beam services.
   * @tparam B The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ProtocolServiceLocatorClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = dereference_t<B>;

      /**
       * Constructs a ProtocolServiceLocatorClient.
       * @param username The username.
       * @param password The password.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       * @throws AuthenticationException If the username or password is invalid.
       */
      template<Initializes<B> BF>
      ProtocolServiceLocatorClient(
        std::string username, std::string password, BF&& client_builder);

      /**
       * Constructs a ProtocolServiceLocatorClient from an existing session.
       * @param session_id The encrypted session id.
       * @param key The encryption key used to encode the session id.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       * @throws AuthenticationException If the session is invalid.
       */
      template<Initializes<B> BF>
      ProtocolServiceLocatorClient(
        const std::string& session_id, unsigned int key, BF&& client_builder);

      ~ProtocolServiceLocatorClient();

      DirectoryEntry get_account() const;
      std::string get_session_id() const;
      std::string get_encrypted_session_id(unsigned int key) const;
      DirectoryEntry authenticate_account(
        const std::string& username, const std::string& password);
      DirectoryEntry authenticate_session(
        const std::string& session_id, unsigned int key);
      std::vector<ServiceEntry> locate(const std::string& name);
      ServiceEntry add(const std::string& name, const JsonObject& properties);
      void remove(const ServiceEntry& service);
      std::vector<DirectoryEntry> load_all_accounts();
      boost::optional<DirectoryEntry> find_account(const std::string& name);
      DirectoryEntry make_account(const std::string& name,
        const std::string& password, const DirectoryEntry& parent);
      DirectoryEntry make_directory(
        const std::string& name, const DirectoryEntry& parent);
      void store_password(
        const DirectoryEntry& account, const std::string& password);
      void monitor(ScopedQueueWriter<AccountUpdate> queue);
      void monitor(
        const std::string& name, ScopedQueueWriter<ServiceUpdate> queue);
      DirectoryEntry load_directory_entry(
        const DirectoryEntry& root, const std::string& path);
      DirectoryEntry load_directory_entry(unsigned int id);
      std::vector<DirectoryEntry> load_parents(const DirectoryEntry& entry);
      std::vector<DirectoryEntry> load_children(const DirectoryEntry& entry);
      void remove(const DirectoryEntry& entry);
      void associate(const DirectoryEntry& entry, const DirectoryEntry& parent);
      void detach(const DirectoryEntry& entry, const DirectoryEntry& parent);
      bool has_permissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions);
      void store(const DirectoryEntry& source, const DirectoryEntry& target,
        Permissions permissions);
      boost::posix_time::ptime load_registration_time(
        const DirectoryEntry& account);
      boost::posix_time::ptime load_last_login_time(
        const DirectoryEntry& account);
      DirectoryEntry rename(
        const DirectoryEntry& entry, const std::string& name);
      void close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      struct ServiceSubscription {
        std::uint64_t m_generation = 0;
        std::vector<ServiceEntry> m_snapshot;
        QueueWriterPublisher<ServiceUpdate> m_publisher;
      };
      struct ServiceRegistration {
        ServiceEntry m_service;
        std::uint64_t m_generation;
      };
      mutable boost::mutex m_mutex;
      ServiceProtocolClient* m_connection;
      std::uint64_t m_generation;
      std::string m_username;
      std::string m_password;
      ServiceProtocolClientHandler<B> m_client_handler;
      std::string m_session_id;
      DirectoryEntry m_account;
      std::vector<DirectoryEntry> m_account_update_snapshot;
      std::optional<QueueWriterPublisher<AccountUpdate>>
        m_account_update_publisher;
      SynchronizedVector<ServiceRegistration> m_services;
      std::unordered_map<std::string, ServiceSubscription> m_subscriptions;
      std::exception_ptr m_exception;
      RoutineTaskQueue m_tasks;
      OpenState m_open_state;

      ProtocolServiceLocatorClient(
        const ProtocolServiceLocatorClient&) = delete;
      ProtocolServiceLocatorClient& operator =(
        const ProtocolServiceLocatorClient&) = delete;
      std::uint64_t get_generation(const ServiceProtocolClient& client) const;
      void login(ServiceProtocolClient& client);
      void login_from_session(ServiceProtocolClient& client,
        const std::string& session_id, unsigned int key);
      void subscribe(const std::string& name, ServiceSubscription& subscription,
        const std::shared_ptr<ServiceProtocolClient>& client);
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void on_account_update(
        ServiceProtocolClient& client, const AccountUpdate& update);
      void on_service_availability(ServiceProtocolClient& client,
        const ServiceEntry& service, bool is_available);
  };

  template<typename B>
  template<Initializes<B> BF>
  ProtocolServiceLocatorClient<B>::ProtocolServiceLocatorClient(
      std::string username, std::string password, BF&& client_builder)
      try : m_connection(nullptr),
            m_generation(0),
            m_username(std::move(username)),
            m_password(std::move(password)),
            m_client_handler(std::forward<BF>(client_builder), std::bind_front(
              &ProtocolServiceLocatorClient::on_reconnect, this)),
            m_account_update_publisher(std::in_place) {
    ServiceLocatorServices::register_service_locator_services(
      out(m_client_handler.get_slots()));
    ServiceLocatorServices::register_service_locator_messages(
      out(m_client_handler.get_slots()));
    add_message_slot<ServiceLocatorServices::AccountUpdateMessage>(
      out(m_client_handler.get_slots()),
      std::bind_front(&ProtocolServiceLocatorClient::on_account_update, this));
    add_message_slot<ServiceLocatorServices::ServiceAvailabilityMessage>(
      out(m_client_handler.get_slots()), std::bind_front(
        &ProtocolServiceLocatorClient::on_service_availability, this));
    try {
      auto client = m_client_handler.get_client();
      login(*client);
    } catch(const std::exception&) {
      close();
      throw;
    }
  } catch(const AuthenticationException&) {
    throw;
  } catch(const std::exception&) {
    throw_nested_with_location(
      ConnectException("Failed to login to service locator."));
  }

  template<typename B>
  template<Initializes<B> BF>
  ProtocolServiceLocatorClient<B>::ProtocolServiceLocatorClient(
      const std::string& session_id, unsigned int key, BF&& client_builder)
      try : m_connection(nullptr),
            m_generation(0),
            m_client_handler(std::forward<BF>(client_builder), std::bind_front(
              &ProtocolServiceLocatorClient::on_reconnect, this)),
            m_account_update_publisher(std::in_place) {
    ServiceLocatorServices::register_service_locator_services(
      out(m_client_handler.get_slots()));
    ServiceLocatorServices::register_service_locator_messages(
      out(m_client_handler.get_slots()));
    add_message_slot<ServiceLocatorServices::AccountUpdateMessage>(
      out(m_client_handler.get_slots()),
      std::bind_front(&ProtocolServiceLocatorClient::on_account_update, this));
    add_message_slot<ServiceLocatorServices::ServiceAvailabilityMessage>(
      out(m_client_handler.get_slots()), std::bind_front(
        &ProtocolServiceLocatorClient::on_service_availability, this));
    try {
      auto client = m_client_handler.get_client();
      login_from_session(*client, session_id, key);
    } catch(const std::exception&) {
      close();
      throw;
    }
  } catch(const AuthenticationException&) {
    throw;
  } catch(const std::exception&) {
    throw_nested_with_location(
      ConnectException("Failed to login to service locator."));
  }

  template<typename B>
  ProtocolServiceLocatorClient<B>::~ProtocolServiceLocatorClient() {
    close();
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::get_account() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account;
  }

  template<typename B>
  std::string ProtocolServiceLocatorClient<B>::get_session_id() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_session_id;
  }

  template<typename B>
  std::string ProtocolServiceLocatorClient<B>::get_encrypted_session_id(
      unsigned int key) const {
    auto lock = boost::lock_guard(m_mutex);
    return compute_sha(std::to_string(key) + m_session_id);
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::authenticate_account(
      const std::string& username, const std::string& password) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::AuthenticateAccountService>(username, password);
    }, "Error authenticating account: " + username);
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::
      authenticate_session(const std::string& session_id, unsigned int key) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::SessionAuthenticationService>(session_id, key);
    }, "Error authenticating session: (" + session_id + ", " +
      std::to_string(key) + ")");
  }

  template<typename B>
  std::vector<ServiceEntry> ProtocolServiceLocatorClient<B>::locate(
      const std::string& name) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LocateService>(name);
    }, "Error locating service: " + name);
  }

  template<typename B>
  ServiceEntry ProtocolServiceLocatorClient<B>::add(
      const std::string& name, const JsonObject& properties) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      auto generation = get_generation(*client);
      auto service = client->template send_request<
        ServiceLocatorServices::RegisterService>(name, properties);
      m_services.push_back(ServiceRegistration(service, generation));
      return service;
    }, "Error registering service: " + name);
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::remove(const ServiceEntry& service) {
    service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      auto generation = get_generation(*client);
      client->template send_request<ServiceLocatorServices::UnregisterService>(
        service.get_id());
      m_services.erase_if([&] (const auto& registration) {
        return registration.m_generation == generation &&
          registration.m_service.get_id() == service.get_id();
      });
    }, "Error unregistering service: " + service.get_name());
  }

  template<typename B>
  std::vector<DirectoryEntry>
      ProtocolServiceLocatorClient<B>::load_all_accounts() {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LoadAllAccountsService>();
    }, "Error loading all accounts.");
  }

  template<typename B>
  boost::optional<DirectoryEntry> ProtocolServiceLocatorClient<B>::find_account(
      const std::string& name) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::FindAccountService>(name);
    }, "Error finding account: " + name);
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::MakeAccountService>(name, password, parent);
    }, "Error making account: " + name);
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::MakeDirectoryService>(name, parent);
    }, "Error making directory: " + name);
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::store_password(
      const DirectoryEntry& account, const std::string& password) {
    service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<
        ServiceLocatorServices::StorePasswordService>(account, password);
    }, "Error storing password for account: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::monitor(
      ScopedQueueWriter<AccountUpdate> queue) {
    m_tasks.push([this, queue =
        std::make_shared<ScopedQueueWriter<AccountUpdate>>(std::move(queue))] {
      if(m_exception) {
        queue->close(m_exception);
        return;
      }
      if(m_account_update_publisher->get_size() != 0) {
        try {
          for(auto& account : m_account_update_snapshot) {
            queue->push(AccountUpdate::add(account));
          }
          m_account_update_publisher->monitor(std::move(*queue));
        } catch(const std::exception&) {}
        return;
      }
      try {
        auto client = m_client_handler.get_client();
        m_account_update_snapshot = client->template send_request<
          ServiceLocatorServices::MonitorAccountsService>();
      } catch(const std::exception&) {
        queue->close(make_nested_service_exception("monitor accounts failed."));
        return;
      }
      m_account_update_publisher->monitor(std::move(*queue));
      for(auto& account : m_account_update_snapshot) {
        m_account_update_publisher->push(AccountUpdate::add(account));
      }
    });
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::monitor(
      const std::string& name, ScopedQueueWriter<ServiceUpdate> queue) {
    m_tasks.push([=, this, queue =
        std::make_shared<ScopedQueueWriter<ServiceUpdate>>(std::move(queue))] {
      if(m_exception) {
        queue->close(m_exception);
        return;
      }
      auto& subscription = m_subscriptions[name];
      try {
        for(auto& service : subscription.m_snapshot) {
          queue->push(ServiceUpdate::add(service));
        }
        subscription.m_publisher.monitor(std::move(*queue));
      } catch(const std::exception&) {
        return;
      }
      try {
        subscribe(name, subscription, m_client_handler.get_client());
      } catch(const IOException&) {
      } catch(const std::exception&) {
        subscription.m_publisher.close(
          make_nested_service_exception("Error monitoring service: " + name));
        m_subscriptions.erase(name);
      }
    });
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::load_directory_entry(
      const DirectoryEntry& root, const std::string& path) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LoadPathService>(root, path);
    }, "Error loading directory entry path: " +
      boost::lexical_cast<std::string>(root) + ", " + path);
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::load_directory_entry(
      unsigned int id) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LoadDirectoryEntryService>(id);
    }, "Error loading directory entry: " + std::to_string(id));
  }

  template<typename B>
  std::vector<DirectoryEntry> ProtocolServiceLocatorClient<B>::load_parents(
      const DirectoryEntry& entry) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LoadParentsService>(entry);
    }, "Error loading parents: " + boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  std::vector<DirectoryEntry> ProtocolServiceLocatorClient<B>::load_children(
      const DirectoryEntry& entry) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LoadChildrenService>(entry);
    }, "Error loading children: " + boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::remove(const DirectoryEntry& entry) {
    service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<
        ServiceLocatorServices::DeleteDirectoryEntryService>(entry);
    }, "Error deleting: " + boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<ServiceLocatorServices::AssociateService>(
        entry, parent);
    }, "Error associating: " + boost::lexical_cast<std::string>(entry) + ", " +
      boost::lexical_cast<std::string>(parent));
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<ServiceLocatorServices::DetachService>(
        entry, parent);
    }, "Error detaching: " + boost::lexical_cast<std::string>(entry) + ", " +
      boost::lexical_cast<std::string>(parent));
  }

  template<typename B>
  bool ProtocolServiceLocatorClient<B>::has_permissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::HasPermissionsService>(
          account, target, permissions);
    }, "Error checking permissions: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(target));
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::store(const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<
        ServiceLocatorServices::StorePermissionsService>(
          source, target, permissions);
    }, "Error storing permissions: " +
      boost::lexical_cast<std::string>(source) + ", " +
      boost::lexical_cast<std::string>(target));
  }

  template<typename B>
  boost::posix_time::ptime
      ProtocolServiceLocatorClient<B>::load_registration_time(
        const DirectoryEntry& account) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LoadRegistrationTimeService>(account);
    }, "Error loading registration time: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  boost::posix_time::ptime
      ProtocolServiceLocatorClient<B>::load_last_login_time(
        const DirectoryEntry& account) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::LoadLastLoginTimeService>(account);
    }, "Error loading last login time: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  DirectoryEntry ProtocolServiceLocatorClient<B>::rename(
      const DirectoryEntry& entry, const std::string& name) {
    return service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ServiceLocatorServices::RenameService>(entry, name);
    }, "Error renaming: " + boost::lexical_cast<std::string>(entry) + ", " +
      name);
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_client_handler.close();
    m_tasks.wait();
    m_account_update_publisher->close();
    m_subscriptions.clear();
    m_open_state.close();
  }

  template<typename B>
  std::uint64_t ProtocolServiceLocatorClient<B>::get_generation(
      const ServiceProtocolClient& client) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_connection != &client) {
      return 0;
    }
    return m_generation;
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::login(ServiceProtocolClient& client) {
    auto result = [&] {
      try {
        return client.template send_request<
          ServiceLocatorServices::LoginService>(m_username, m_password);
      } catch(const ServiceRequestException& exception) {
        if(std::string_view(exception.what()) ==
            "Invalid username or password.") {
          boost::throw_with_location(AuthenticationException(exception.what()));
        }
        throw;
      }
    }();
    auto lock = boost::lock_guard(m_mutex);
    m_account = result.account;
    m_session_id = result.session_id;
    if(m_generation == 0) {
      m_connection = &client;
      ++m_generation;
    }
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::login_from_session(
      ServiceProtocolClient& client, const std::string& session_id,
      unsigned int key) {
    auto result = [&] {
      try {
        return client.template send_request<
          ServiceLocatorServices::LoginFromSessionService>(session_id, key);
      } catch(const ServiceRequestException& exception) {
        if(std::string_view(exception.what()) == "Session not found.") {
          boost::throw_with_location(AuthenticationException(exception.what()));
        }
        throw;
      }
    }();
    auto lock = boost::lock_guard(m_mutex);
    m_account = result.account;
    m_session_id = result.session_id;
    if(m_generation == 0) {
      m_connection = &client;
      ++m_generation;
    }
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::subscribe(const std::string& name,
      ServiceSubscription& subscription,
      const std::shared_ptr<ServiceProtocolClient>& client) {
    auto generation = get_generation(*client);
    if(generation == 0 || subscription.m_generation == generation) {
      return;
    }
    auto snapshot = std::vector<ServiceEntry>();
    try {
      snapshot = client->template send_request<
        ServiceLocatorServices::SubscribeAvailabilityService>(name);
    } catch(const ServiceRequestException& exception) {
      if(std::string(exception.what()) == "ServiceProtocolClient closed.") {
        return;
      }
      throw;
    }
    for(auto& service : subscription.m_snapshot) {
      if(!std::ranges::contains(snapshot, service)) {
        subscription.m_publisher.push(ServiceUpdate::remove(service));
      }
    }
    for(auto& service : snapshot) {
      if(!std::ranges::contains(subscription.m_snapshot, service)) {
        subscription.m_publisher.push(ServiceUpdate::add(service));
      }
    }
    subscription.m_snapshot = std::move(snapshot);
    subscription.m_generation = generation;
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_connection = client.get();
      ++m_generation;
    }
    try {
      login(*client);
    } catch(const std::exception&) {
      auto exception =
        make_nested_service_exception("Error reconnecting to service locator.");
      m_tasks.push([=, this] {
        m_exception = exception;
        m_account_update_publisher->close(exception);
        for(auto& subscription :
            m_subscriptions | boost::adaptors::map_values) {
          subscription.m_publisher.close(exception);
        }
        m_subscriptions.clear();
      });
      throw;
    }
    m_tasks.push([=, this] {
      auto generation = get_generation(*client);
      if(generation == 0) {
        return;
      }
      auto services = m_services.with([&] (auto& registrations) {
        auto services = std::vector<ServiceRegistration>();
        std::erase_if(registrations, [&] (const auto& registration) {
          if(registration.m_generation >= generation) {
            return false;
          }
          services.push_back(registration);
          return true;
        });
        return services;
      });
      auto i = m_subscriptions.begin();
      while(i != m_subscriptions.end()) {
        if(i->second.m_publisher.get_size() == 0) {
          i = m_subscriptions.erase(i);
          continue;
        }
        try {
          subscribe(i->first, i->second, client);
          ++i;
        } catch(const IOException&) {
          ++i;
        } catch(const std::exception&) {
          i->second.m_publisher.close(make_nested_service_exception(
            "Error monitoring service: " + i->first));
          i = m_subscriptions.erase(i);
        }
      }
      for(auto& service : services) {
        try {
          auto registration = client->template send_request<
            ServiceLocatorServices::RegisterService>(
              service.m_service.get_name(), service.m_service.get_properties());
          m_services.push_back(
            ServiceRegistration(std::move(registration), generation));
        } catch(const IOException&) {
          m_services.push_back(service);
        } catch(const ServiceRequestException& exception) {
          m_services.push_back(service);
          if(std::string(exception.what()) != "ServiceProtocolClient closed.") {
            std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
          }
        } catch(const std::exception&) {
          m_services.push_back(service);
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
      }
      if(m_account_update_publisher->get_size() == 0) {
        return;
      }
      auto accounts = std::vector<DirectoryEntry>();
      try {
        accounts = client->template send_request<
          ServiceLocatorServices::MonitorAccountsService>();
      } catch(const IOException&) {
        return;
      } catch(const ServiceRequestException& exception) {
        if(std::string(exception.what()) != "ServiceProtocolClient closed.") {
          m_account_update_publisher->close(
            make_nested_service_exception("Error monitoring accounts."));
          m_account_update_publisher.emplace();
          m_account_update_snapshot.clear();
        }
        return;
      } catch(const std::exception&) {
        m_account_update_publisher->close(
          make_nested_service_exception("Error monitoring accounts."));
        m_account_update_publisher.emplace();
        m_account_update_snapshot.clear();
        return;
      }
      for(auto& account : accounts) {
        auto i = std::find(m_account_update_snapshot.begin(),
          m_account_update_snapshot.end(), account);
        if(i == m_account_update_snapshot.end()) {
          m_account_update_snapshot.push_back(account);
          m_account_update_publisher->push(AccountUpdate::add(account));
        }
      }
    });
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::on_account_update(
      ServiceProtocolClient& client, const AccountUpdate& update) {
    m_tasks.push([=, this] {
      if(update.m_type == AccountUpdate::Type::ADDED) {
        m_account_update_snapshot.push_back(update.m_account);
      } else {
        auto i = std::find(m_account_update_snapshot.begin(),
          m_account_update_snapshot.end(), update.m_account);
        if(i != m_account_update_snapshot.end()) {
          m_account_update_snapshot.erase(i);
        }
      }
      m_account_update_publisher->push(update);
      if(m_account_update_publisher->get_size() == 0) {
        m_account_update_snapshot = {};
        try {
          auto client = m_client_handler.get_client();
          client->template send_request<
            ServiceLocatorServices::UnmonitorAccountsService>();
        } catch(const std::exception&) {}
      }
    });
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::on_service_availability(
      ServiceProtocolClient& client, const ServiceEntry& service,
      bool is_available) {
    auto generation = get_generation(client);
    if(generation == 0) {
      return;
    }
    m_tasks.push([=, this] {
      auto i = m_subscriptions.find(service.get_name());
      if(i == m_subscriptions.end() || i->second.m_generation != generation) {
        return;
      }
      auto& subscription = i->second;
      auto entry = std::ranges::find(
        subscription.m_snapshot, service.get_id(), &ServiceEntry::get_id);
      if(is_available) {
        if(entry == subscription.m_snapshot.end()) {
          subscription.m_snapshot.push_back(service);
          subscription.m_publisher.push(ServiceUpdate::add(service));
        } else if(*entry != service) {
          subscription.m_publisher.push(ServiceUpdate::remove(*entry));
          *entry = service;
          subscription.m_publisher.push(ServiceUpdate::add(service));
        }
      } else if(entry != subscription.m_snapshot.end() && *entry == service) {
        subscription.m_publisher.push(ServiceUpdate::remove(*entry));
        subscription.m_snapshot.erase(entry);
      }
    });
  }
}

#endif
