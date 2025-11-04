#ifndef BEAM_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_SERVICE_LOCATOR_CLIENT_HPP
#include <concepts>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/ServiceLocator/AccountUpdate.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam {

  /** Concept for types that can be used as a ServiceLocatorClient. */
  template<typename T>
  concept IsServiceLocatorClient = IsConnection<T> && requires(
      T client, const T cclient) {
    { cclient.get_account() } -> std::same_as<DirectoryEntry>;
    { cclient.get_session_id() } -> std::same_as<std::string>;
    { cclient.get_encrypted_session_id(std::declval<unsigned int>()) } ->
        std::same_as<std::string>;
    { client.authenticate_account(std::declval<const std::string&>(),
        std::declval<const std::string&>()) } -> std::same_as<DirectoryEntry>;
    { client.authenticate_session(std::declval<const std::string&>(),
        std::declval<unsigned int>()) } -> std::same_as<DirectoryEntry>;
    { client.locate(std::declval<const std::string&>()) } ->
        std::same_as<std::vector<ServiceEntry>>;
    { client.add(std::declval<const std::string&>(),
        std::declval<const JsonObject&>()) } -> std::same_as<ServiceEntry>;
    { client.remove(std::declval<const ServiceEntry&>()) } ->
        std::same_as<void>;
    { client.load_all_accounts() } -> std::same_as<std::vector<DirectoryEntry>>;
    { client.find_account(std::declval<const std::string&>()) } ->
        std::same_as<boost::optional<DirectoryEntry>>;
    { client.make_account(std::declval<const std::string&>(),
        std::declval<const std::string&>(),
        std::declval<const DirectoryEntry&>()) } ->
          std::same_as<DirectoryEntry>;
    { client.make_directory(std::declval<const std::string&>(),
        std::declval<const DirectoryEntry&>()) } ->
          std::same_as<DirectoryEntry>;
    { client.store_password(std::declval<const DirectoryEntry&>(),
        std::declval<const std::string&>()) } -> std::same_as<void>;
    { client.monitor(std::declval<ScopedQueueWriter<AccountUpdate>>()) } ->
        std::same_as<void>;
    { client.load_directory_entry(std::declval<const DirectoryEntry&>(),
        std::declval<const std::string&>()) } -> std::same_as<DirectoryEntry>;
    { client.load_directory_entry(std::declval<unsigned int>()) } ->
        std::same_as<DirectoryEntry>;
    { client.load_parents(std::declval<const DirectoryEntry&>()) } ->
        std::same_as<std::vector<DirectoryEntry>>;
    { client.load_children(std::declval<const DirectoryEntry&>()) } ->
        std::same_as<std::vector<DirectoryEntry>>;
    { client.remove(std::declval<const DirectoryEntry&>()) } ->
        std::same_as<void>;
    { client.associate(std::declval<const DirectoryEntry&>(),
        std::declval<const DirectoryEntry&>()) } -> std::same_as<void>;
    { client.detach(std::declval<const DirectoryEntry&>(),
        std::declval<const DirectoryEntry&>()) } -> std::same_as<void>;
    { client.has_permissions(std::declval<const DirectoryEntry&>(),
        std::declval<const DirectoryEntry&>(), std::declval<Permissions>()) } ->
          std::same_as<bool>;
    { client.store(std::declval<const DirectoryEntry&>(),
        std::declval<const DirectoryEntry&>(),
        std::declval<Permissions>()) } -> std::same_as<void>;
    { client.load_registration_time(std::declval<const DirectoryEntry&>()) } ->
        std::same_as<boost::posix_time::ptime>;
    { client.load_last_login_time(std::declval<const DirectoryEntry&>()) } ->
        std::same_as<boost::posix_time::ptime>;
    { client.rename(std::declval<const DirectoryEntry&>(),
        std::declval<const std::string&>()) } -> std::same_as<DirectoryEntry>;
    { client.close() } -> std::same_as<void>;
  };

  /** Client used to locate services on a service network. */
  class ServiceLocatorClient {
    public:

      /**
       * Constructs a ServiceLocatorClient of a specified type using
       * emplacement.
       * @tparam T The type of client to emplace.
       * @param args The arguments to pass to the emplaced client.
       */
      template<IsServiceLocatorClient T, typename... Args>
      explicit ServiceLocatorClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ServiceLocatorClient by referencing an existing client.
       * @param client The client to reference.
       */
      template<DisableCopy<ServiceLocatorClient> T> requires
        IsServiceLocatorClient<dereference_t<T>>
      ServiceLocatorClient(T&& client);

      ServiceLocatorClient(const ServiceLocatorClient&) = default;
      ServiceLocatorClient(ServiceLocatorClient&&) = default;

      /** Returns the DirectoryEntry of the account that's logged in. */
      DirectoryEntry get_account() const;

      /** Returns the raw session id. */
      std::string get_session_id() const;

      /**
       * Encrypts the session id.
       * @param key The encryption key.
       * @return The session id encrypted using the specified encryption
       *         <i>key</i>.
       */
      std::string get_encrypted_session_id(unsigned int key) const;

      /**
       * Authenticates an account.
       * @param name The username of the account.
       * @param password The account's password.
       * @return DirectoryEntry The account's DirectoryEntry iff the username
       *         and password match.
       */
      DirectoryEntry authenticate_account(
        const std::string& username, const std::string& password);

      /**
       * Authenticates a session.
       * @param sessionId An encoded session id.
       * @param key The encryption key used to encode the <i>sessionId</i>.
       * @return The DirectoryEntry of the account using the specified
       *        <i>sessionId</i>.
       */
      DirectoryEntry authenticate_session(
        const std::string& session_id, unsigned int key);

      /**
       * Locates a service.
       * @param name The name of the service to locate.
       * @return The list of services that were located.
       */
      std::vector<ServiceEntry> locate(const std::string& name);

      /**
       * Registers a service.
       * @param name The name of the service.
       * @param properties The service's properties.
       * @return The ServiceEntry corresponding to the registered service.
       */
      ServiceEntry add(const std::string& name, const JsonObject& properties);

      /**
       * Unregisters a service.
       * @param service The service to unregister.
       */
      void remove(const ServiceEntry& service);

      /** Loads the list of all accounts this client is allowed to read. */
      std::vector<DirectoryEntry> load_all_accounts();

      /**
       * Finds an account with a specified name.
       * @param name The name of the account to find.
       * @return The DirectoryEntry of the account with the given <i>name</i>.
       */
      boost::optional<DirectoryEntry> find_account(const std::string& name);

      /**
       * Creates an account.
       * @param name The name of the account.
       * @param password The account's password.
       * @param parent The parent to place the account in.
       * @return The DirectoryEntry of the account that was created.
       */
      DirectoryEntry make_account(const std::string& name,
        const std::string& password, const DirectoryEntry& parent);

      /**
       * Creates a directory.
       * @param name The name of the directory.
       * @param parent The parent to place the directory in.
       * @return The DirectoryEntry of the directory that was created.
       */
      DirectoryEntry make_directory(
        const std::string& name, const DirectoryEntry& parent);

      /**
       * Sets an account's password.
       * @param account The account to set the password for.
       * @param password The password to set.
       */
      void store_password(
        const DirectoryEntry& account, const std::string& password);

      /**
       * Monitors new and deleted accounts and pushes them to a queue.
       * @param queue The queue to push to.
       */
      void monitor(ScopedQueueWriter<AccountUpdate> queue);

      /**
       * Loads a DirectoryEntry from a path.
       * @param root The root DirectoryEntry to begin searching from.
       * @param path The path of the DirectoryEntry to load.
       * @return The DirectoryEntry with the specified <i>path</i> from the
       *         <i>root</i>.
       */
      DirectoryEntry load_directory_entry(
        const DirectoryEntry& root, const std::string& path);

      /**
       * Loads a DirectoryEntry from an id.
       * @param id The id of the DirectoryEntry to load.
       * @return The DirectoryEntry with the specified <i>id</i>.
       */
      DirectoryEntry load_directory_entry(unsigned int id);

      /**
       * Loads all parents of a specified DirectoryEntry.
       * @param entry The DirectoryEntry whose parents are to be loaded.
       * @return The list of parents of the specified <i>entry</i>.
       */
      std::vector<DirectoryEntry> load_parents(const DirectoryEntry& entry);

      /**
       * Loads all children of a specified DirectoryEntry.
       * @param entry The DirectoryEntry whose children are to be loaded.
       * @return The list of children of the specified <i>entry</i>.
       */
      std::vector<DirectoryEntry> load_children(const DirectoryEntry& entry);

      /**
       * Deletes a DirectoryEntry.
       * @param entry The DirectoryEntry to delete.
       */
      void remove(const DirectoryEntry& entry);

      /**
       * Associates a DirectoryEntry with a parent.
       * @param entry The DirectoryEntry to associate.
       * @param parent The parent to associate with the <i>entry</i>.
       */
      void associate(const DirectoryEntry& entry, const DirectoryEntry& parent);

      /**
       * Detaches a DirectoryEntry from a parent.
       * @param entry The DirectoryEntry to detach.
       * @param parent The parent to detach from the <i>entry</i>.
       */
      void detach(const DirectoryEntry& entry, const DirectoryEntry& parent);

      /**
       * Tests if an account has Permissions to a DirectoryEntry.
       * @param account The account to test.
       * @param target The DirectoryEntry to test.
       * @param permissions The Permissions to test.
       * @return <code>true</code> iff the <i>account</i> has the specified
       *         <code>permissions</code> on the <i>target</i>.
       */
      bool has_permissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions);

      /**
       * Sets a DirectoryEntry's Permissions.
       * @param source The DirectoryEntry to grant the Permissions to.
       * @param target The DirectoryEntry to grant the Permissions over.
       * @param permissions The Permissions to grant to the <i>source</i> over
       *        the <i>target</i>.
       */
      void store(const DirectoryEntry& source, const DirectoryEntry& target,
        Permissions permissions);

      /**
       * Loads an account's registration time.
       * @param account The account whose registration time is to be loaded.
       * @return The registration time of the <i>account</i>.
       */
      boost::posix_time::ptime load_registration_time(
        const DirectoryEntry& account);

      /**
       * Loads an account's last login time.
       * @param account The account whose last login time is to be loaded.
       * @return The most recent login time of the <i>account</i>.
       */
      boost::posix_time::ptime load_last_login_time(
        const DirectoryEntry& account);

      /**
       * Renames a DirectoryEntry.
       * @param entry The DirectoryEntry to rename.
       * @param name The name to assign to the DirectoryEntry.
       * @return The updated DirectoryEntry.
       */
      DirectoryEntry rename(
        const DirectoryEntry& entry, const std::string& name);

      void close();

    private:
      struct VirtualServiceLocatorClient {
        virtual ~VirtualServiceLocatorClient() = default;

        virtual DirectoryEntry get_account() const = 0;
        virtual std::string get_session_id() const = 0;
        virtual std::string get_encrypted_session_id(unsigned int) const = 0;
        virtual DirectoryEntry authenticate_account(
          const std::string&, const std::string&) = 0;
        virtual DirectoryEntry authenticate_session(
          const std::string&, unsigned int) = 0;
        virtual std::vector<ServiceEntry> locate(const std::string&) = 0;
        virtual ServiceEntry add(const std::string&, const JsonObject&) = 0;
        virtual void remove(const ServiceEntry&) = 0;
        virtual std::vector<DirectoryEntry> load_all_accounts() = 0;
        virtual boost::optional<DirectoryEntry> find_account(
          const std::string&) = 0;
        virtual DirectoryEntry make_account(
          const std::string&, const std::string&, const DirectoryEntry&) = 0;
        virtual DirectoryEntry make_directory(
          const std::string&, const DirectoryEntry&) = 0;
        virtual void store_password(
          const DirectoryEntry&, const std::string&) = 0;
        virtual void monitor(ScopedQueueWriter<AccountUpdate>) = 0;
        virtual DirectoryEntry load_directory_entry(
          const DirectoryEntry&, const std::string&) = 0;
        virtual DirectoryEntry load_directory_entry(unsigned int) = 0;
        virtual std::vector<DirectoryEntry> load_parents(
          const DirectoryEntry&) = 0;
        virtual std::vector<DirectoryEntry> load_children(
          const DirectoryEntry&) = 0;
        virtual void remove(const DirectoryEntry&) = 0;
        virtual void associate(
          const DirectoryEntry&, const DirectoryEntry&) = 0;
        virtual void detach(const DirectoryEntry&, const DirectoryEntry&) = 0;
        virtual bool has_permissions(
          const DirectoryEntry&, const DirectoryEntry&, Permissions) = 0;
        virtual void store(
          const DirectoryEntry&, const DirectoryEntry&, Permissions) = 0;
        virtual boost::posix_time::ptime load_registration_time(
          const DirectoryEntry&) = 0;
        virtual boost::posix_time::ptime load_last_login_time(
          const DirectoryEntry&) = 0;
        virtual DirectoryEntry rename(
          const DirectoryEntry&, const std::string&) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedServiceLocatorClient final : VirtualServiceLocatorClient {
        using ServiceLocatorClient = C;
        local_ptr_t<ServiceLocatorClient> m_client;

        template<typename... Args>
        WrappedServiceLocatorClient(Args&&... args);

        DirectoryEntry get_account() const override;
        std::string get_session_id() const override;
        std::string get_encrypted_session_id(unsigned int key) const override;
        DirectoryEntry authenticate_account(
          const std::string& username, const std::string& password) override;
        DirectoryEntry authenticate_session(
          const std::string& session_id, unsigned int key) override;
        std::vector<ServiceEntry> locate(const std::string& name) override;
        ServiceEntry add(
          const std::string& name, const JsonObject& properties) override;
        void remove(const ServiceEntry& service) override;
        std::vector<DirectoryEntry> load_all_accounts() override;
        boost::optional<DirectoryEntry> find_account(
          const std::string& name) override;
        DirectoryEntry make_account(const std::string& name,
          const std::string& password, const DirectoryEntry& parent) override;
        DirectoryEntry make_directory(
          const std::string& name, const DirectoryEntry& parent) override;
        void store_password(
          const DirectoryEntry& account, const std::string& password) override;
        void monitor(ScopedQueueWriter<AccountUpdate> queue) override;
        DirectoryEntry load_directory_entry(
          const DirectoryEntry& root, const std::string& path) override;
        DirectoryEntry load_directory_entry(unsigned int id) override;
        std::vector<DirectoryEntry> load_parents(
          const DirectoryEntry& entry) override;
        std::vector<DirectoryEntry> load_children(
          const DirectoryEntry& entry) override;
        void remove(const DirectoryEntry& entry) override;
        void associate(
          const DirectoryEntry& entry, const DirectoryEntry& parent) override;
        void detach(
          const DirectoryEntry& entry, const DirectoryEntry& parent) override;
        bool has_permissions(const DirectoryEntry& account,
          const DirectoryEntry& target, Permissions permissions) override;
        void store(const DirectoryEntry& source, const DirectoryEntry& target,
          Permissions permissions) override;
        boost::posix_time::ptime load_registration_time(
          const DirectoryEntry& account) override;
        boost::posix_time::ptime load_last_login_time(
          const DirectoryEntry& account) override;
        DirectoryEntry rename(
          const DirectoryEntry& entry, const std::string& name) override;
        void close() override;
      };
      VirtualPtr<VirtualServiceLocatorClient> m_client;
  };

  /**
   * Loads a directory, or creates it if it doesn't already exist.
   * @param serviceLocatorClient The ServiceLocatorClient to use.
   * @param name The name of the directory to load or create.
   * @param parent The directory's parent.
   * @return directory The directory that was loaded.
   */
  template<IsServiceLocatorClient C>
  DirectoryEntry load_or_create_directory(
      C& client, const std::string& name, const DirectoryEntry& parent) {
    try {
      return client.load_directory_entry(parent, name);
    } catch(const ServiceRequestException&) {
      return client.make_directory(name, parent);
    }
  }

  /**
   * Locates the IP addresses of a service.
   * @param client The ServiceLocatorClient used to locate the addresses.
   * @param name The name of the service to locate.
   * @param predicate A function to apply to a ServiceEntry to determine if it
   *        matches some criteria.
   * @return The list of IP addresses for the specified service.
   */
  template<IsServiceLocatorClient C, typename F>
  std::vector<IpAddress> locate_service_addresses(
      C& client, const std::string& name, F predicate) {
    auto services = std::vector<ServiceEntry>();
    try {
      services = client.locate(name);
    } catch(const std::exception&) {
      boost::throw_with_location(
        ConnectException("No " + name + " services available."));
    }
    std::erase_if(services, [&] (const auto& entry) {
      return !predicate(entry);
    });
    if(services.empty()) {
      boost::throw_with_location(
        ConnectException("No " + name + " services available."));
    }
    auto seed = std::random_device();
    auto generator = std::mt19937(seed());
    auto distribution =
      std::uniform_int_distribution<std::size_t>(0, services.size() - 1);
    auto& service = services[distribution(generator)];
    auto addresses = parse<std::vector<IpAddress>>(
      boost::get<std::string>(service.get_properties().at("addresses")));
    return addresses;
  }

  /**
   * Locates the IP addresses of a service.
   * @param client The ServiceLocatorClient used to locate the addresses.
   * @param name The name of the service to locate.
   * @return The list of IP addresses for the specified service.
   */
  template<IsServiceLocatorClient C>
  std::vector<IpAddress> locate_service_addresses(
      C& client, const std::string& name) {
    return locate_service_addresses(client, name, [] (auto&&) {
      return true;
    });
  }

  template<IsServiceLocatorClient T, typename... Args>
  ServiceLocatorClient::ServiceLocatorClient(
    std::in_place_type_t<T>, Args&&... args)
    : m_client(make_virtual_ptr<WrappedServiceLocatorClient<T>>(
        std::forward<Args>(args)...)) {}

  template<DisableCopy<ServiceLocatorClient> T> requires
    IsServiceLocatorClient<dereference_t<T>>
  ServiceLocatorClient::ServiceLocatorClient(T&& client)
    : m_client(make_virtual_ptr<WrappedServiceLocatorClient<
        std::remove_cvref_t<T>>>(std::forward<T>(client))) {}

  inline DirectoryEntry ServiceLocatorClient::get_account() const {
    return m_client->get_account();
  }

  inline std::string ServiceLocatorClient::get_session_id() const {
    return m_client->get_session_id();
  }

  inline std::string ServiceLocatorClient::get_encrypted_session_id(
      unsigned int key) const {
    return m_client->get_encrypted_session_id(key);
  }

  inline DirectoryEntry ServiceLocatorClient::authenticate_account(
      const std::string& username, const std::string& password) {
    return m_client->authenticate_account(username, password);
  }

  inline DirectoryEntry ServiceLocatorClient::authenticate_session(
      const std::string& session_id, unsigned int key) {
    return m_client->authenticate_session(session_id, key);
  }

  inline std::vector<ServiceEntry> ServiceLocatorClient::locate(
      const std::string& name) {
    return m_client->locate(name);
  }

  inline ServiceEntry ServiceLocatorClient::add(
      const std::string& name, const JsonObject& properties) {
    return m_client->add(name, properties);
  }

  inline void ServiceLocatorClient::remove(const ServiceEntry& service) {
    m_client->remove(service);
  }

  inline std::vector<DirectoryEntry> ServiceLocatorClient::load_all_accounts() {
    return m_client->load_all_accounts();
  }

  inline boost::optional<DirectoryEntry> ServiceLocatorClient::find_account(
      const std::string& name) {
    return m_client->find_account(name);
  }

  inline DirectoryEntry ServiceLocatorClient::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    return m_client->make_account(name, password, parent);
  }

  inline DirectoryEntry ServiceLocatorClient::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    return m_client->make_directory(name, parent);
  }

  inline void ServiceLocatorClient::store_password(
      const DirectoryEntry& account, const std::string& password) {
    m_client->store_password(account, password);
  }

  inline void ServiceLocatorClient::monitor(
      ScopedQueueWriter<AccountUpdate> queue) {
    m_client->monitor(std::move(queue));
  }

  inline DirectoryEntry ServiceLocatorClient::load_directory_entry(
      const DirectoryEntry& root, const std::string& path) {
    return m_client->load_directory_entry(root, path);
  }

  inline DirectoryEntry ServiceLocatorClient::load_directory_entry(
      unsigned int id) {
    return m_client->load_directory_entry(id);
  }

  inline std::vector<DirectoryEntry> ServiceLocatorClient::load_parents(
      const DirectoryEntry& entry) {
    return m_client->load_parents(entry);
  }

  inline std::vector<DirectoryEntry> ServiceLocatorClient::load_children(
      const DirectoryEntry& entry) {
    return m_client->load_children(entry);
  }

  inline void ServiceLocatorClient::remove(const DirectoryEntry& entry) {
    m_client->remove(entry);
  }

  inline void ServiceLocatorClient::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->associate(entry, parent);
  }

  inline void ServiceLocatorClient::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->detach(entry, parent);
  }

  inline bool ServiceLocatorClient::has_permissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return m_client->has_permissions(account, target, permissions);
  }

  inline void ServiceLocatorClient::store(const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    m_client->store(source, target, permissions);
  }

  inline boost::posix_time::ptime ServiceLocatorClient::load_registration_time(
      const DirectoryEntry& account) {
    return m_client->load_registration_time(account);
  }

  inline boost::posix_time::ptime ServiceLocatorClient::load_last_login_time(
      const DirectoryEntry& account) {
    return m_client->load_last_login_time(account);
  }

  inline DirectoryEntry ServiceLocatorClient::rename(
      const DirectoryEntry& entry, const std::string& name) {
    return m_client->rename(entry, name);
  }

  inline void ServiceLocatorClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  ServiceLocatorClient::WrappedServiceLocatorClient<C>::
    WrappedServiceLocatorClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  DirectoryEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      get_account() const {
    return m_client->get_account();
  }

  template<typename C>
  std::string ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      get_session_id() const {
    return m_client->get_session_id();
  }

  template<typename C>
  std::string ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      get_encrypted_session_id(unsigned int key) const {
    return m_client->get_encrypted_session_id(key);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      authenticate_account(const std::string& username,
        const std::string& password) {
    return m_client->authenticate_account(username, password);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      authenticate_session(const std::string& session_id, unsigned int key) {
    return m_client->authenticate_session(session_id, key);
  }

  template<typename C>
  std::vector<ServiceEntry>
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::locate(
        const std::string& name) {
    return m_client->locate(name);
  }

  template<typename C>
  ServiceEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::add(
      const std::string& name, const JsonObject& properties) {
    return m_client->add(name, properties);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::remove(
      const ServiceEntry& service) {
    m_client->remove(service);
  }

  template<typename C>
  std::vector<DirectoryEntry>
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::
        load_all_accounts() {
    return m_client->load_all_accounts();
  }

  template<typename C>
  boost::optional<DirectoryEntry>
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::find_account(
        const std::string& name) {
    return m_client->find_account(name);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      make_account(const std::string& name, const std::string& password,
        const DirectoryEntry& parent) {
    return m_client->make_account(name, password, parent);
  }

  template<typename C>
  DirectoryEntry
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::make_directory(
        const std::string& name, const DirectoryEntry& parent) {
    return m_client->make_directory(name, parent);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::store_password(
      const DirectoryEntry& account, const std::string& password) {
    m_client->store_password(account, password);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::monitor(
      ScopedQueueWriter<AccountUpdate> queue) {
    m_client->monitor(std::move(queue));
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      load_directory_entry(
        const DirectoryEntry& root, const std::string& path) {
    return m_client->load_directory_entry(root, path);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::
      load_directory_entry(unsigned int id) {
    return m_client->load_directory_entry(id);
  }

  template<typename C>
  std::vector<DirectoryEntry>
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::load_parents(
        const DirectoryEntry& entry) {
    return m_client->load_parents(entry);
  }

  template<typename C>
  std::vector<DirectoryEntry>
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::load_children(
        const DirectoryEntry& entry) {
    return m_client->load_children(entry);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::remove(
      const DirectoryEntry& entry) {
    m_client->remove(entry);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->associate(entry, parent);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_client->detach(entry, parent);
  }

  template<typename C>
  bool ServiceLocatorClient::WrappedServiceLocatorClient<C>::has_permissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return m_client->has_permissions(account, target, permissions);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::store(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_client->store(source, target, permissions);
  }

  template<typename C>
  boost::posix_time::ptime
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::
        load_registration_time(const DirectoryEntry& account) {
    return m_client->load_registration_time(account);
  }

  template<typename C>
  boost::posix_time::ptime
      ServiceLocatorClient::WrappedServiceLocatorClient<C>::
        load_last_login_time(const DirectoryEntry& account) {
    return m_client->load_last_login_time(account);
  }

  template<typename C>
  DirectoryEntry ServiceLocatorClient::WrappedServiceLocatorClient<C>::rename(
      const DirectoryEntry& entry, const std::string& name) {
    return m_client->rename(entry, name);
  }

  template<typename C>
  void ServiceLocatorClient::WrappedServiceLocatorClient<C>::close() {
    m_client->close();
  }
}

#endif
