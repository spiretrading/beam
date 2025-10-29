#ifndef BEAM_PROTOCOL_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_PROTOCOL_SERVICE_LOCATOR_CLIENT_HPP
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
       */
      template<Initializes<B> BF>
      ProtocolServiceLocatorClient(
        std::string username, std::string password, BF&& client_builder);

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
      mutable boost::mutex m_mutex;
      std::string m_username;
      std::string m_password;
      ServiceProtocolClientHandler<B> m_client_handler;
      std::string m_session_id;
      DirectoryEntry m_account;
      std::vector<DirectoryEntry> m_account_update_snapshot;
      QueueWriterPublisher<AccountUpdate> m_account_update_publisher;
      SynchronizedVector<ServiceEntry> m_services;
      RoutineTaskQueue m_tasks;
      OpenState m_open_state;

      ProtocolServiceLocatorClient(
        const ProtocolServiceLocatorClient&) = delete;
      ProtocolServiceLocatorClient& operator =(
        const ProtocolServiceLocatorClient&) = delete;
      void login(ServiceProtocolClient& client);
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void on_account_update(
        ServiceProtocolClient& client, const AccountUpdate& update);
  };

  template<typename B>
  template<Initializes<B> BF>
  ProtocolServiceLocatorClient<B>::ProtocolServiceLocatorClient(
      std::string username, std::string password, BF&& client_builder)
      try : m_username(std::move(username)),
            m_password(std::move(password)),
            m_client_handler(std::forward<BF>(client_builder), std::bind_front(
              &ProtocolServiceLocatorClient::on_reconnect, this)) {
    ServiceLocatorServices::register_service_locator_services(
      out(m_client_handler.get_slots()));
    ServiceLocatorServices::register_service_locator_messages(
      out(m_client_handler.get_slots()));
    add_message_slot<ServiceLocatorServices::AccountUpdateMessage>(
      out(m_client_handler.get_slots()),
      std::bind_front(&ProtocolServiceLocatorClient::on_account_update, this));
    try {
      auto client = m_client_handler.get_client();
      login(*client);
    } catch(const std::exception&) {
      close();
      throw;
    }
  } catch(const std::exception&) {
    std::throw_with_nested(
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
      auto service = client->template send_request<
        ServiceLocatorServices::RegisterService>(name, properties);
      m_services.push_back(service);
      return service;
    }, "Error registering service: " + name);
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::remove(const ServiceEntry& service) {
    service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<ServiceLocatorServices::UnregisterService>(
        service.get_id());
      m_services.erase_if([&] (const auto& s) {
        return s.get_id() == service.get_id();
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
      if(m_account_update_publisher.get_size() != 0) {
        try {
          for(auto& account : m_account_update_snapshot) {
            queue->push(AccountUpdate::add(account));
          }
          m_account_update_publisher.monitor(std::move(*queue));
        } catch(const PipeBrokenException&) {}
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
      m_account_update_publisher.monitor(std::move(*queue));
      for(auto& account : m_account_update_snapshot) {
        m_account_update_publisher.push(AccountUpdate::add(account));
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
    m_account_update_publisher.close();
    m_tasks.close();
    m_tasks.wait();
    m_client_handler.close();
    m_open_state.close();
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::login(ServiceProtocolClient& client) {
    auto result = client.template send_request<
      ServiceLocatorServices::LoginService>(m_username, m_password);
    auto lock = boost::lock_guard(m_mutex);
    m_account = result.account;
    m_session_id = result.session_id;
  }

  template<typename B>
  void ProtocolServiceLocatorClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    login(*client);
    auto services = std::vector<ServiceEntry>();
    m_services.swap(services);
    m_tasks.push([=, this] {
      for(auto& service : services) {
        add(service.get_name(), service.get_properties());
      }
      if(m_account_update_publisher.get_size() == 0) {
        return;
      }
      auto client = m_client_handler.get_client();
      auto accounts = client->template send_request<
        ServiceLocatorServices::MonitorAccountsService>();
      for(auto& account : accounts) {
        auto i = std::find(m_account_update_snapshot.begin(),
          m_account_update_snapshot.end(), account);
        if(i == m_account_update_snapshot.end()) {
          m_account_update_snapshot.push_back(account);
          m_account_update_publisher.push(AccountUpdate::add(account));
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
      m_account_update_publisher.push(update);
      if(m_account_update_publisher.get_size() == 0) {
        auto client = m_client_handler.get_client();
        client->template send_request<
          ServiceLocatorServices::UnmonitorAccountsService>();
        m_account_update_snapshot = {};
      }
    });
  }
}

#endif
