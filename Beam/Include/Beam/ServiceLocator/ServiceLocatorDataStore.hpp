#ifndef BEAM_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_SERVICE_LOCATOR_DATA_STORE_HPP
#include <concepts>
#include <string>
#include <unordered_set>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Utilities/Bcrypt.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept for types that can be used as a ServiceLocator data store. */
  template<typename T>
  concept IsServiceLocatorDataStore = IsConnection<T> &&
    requires(T& data_store) {
      { data_store.load_parents(std::declval<const DirectoryEntry&>()) } ->
          std::same_as<std::vector<DirectoryEntry>>;
      { data_store.load_children(std::declval<const DirectoryEntry&>()) } ->
          std::same_as<std::vector<DirectoryEntry>>;
      { data_store.load_directory_entry(std::declval<unsigned int>()) } ->
          std::same_as<DirectoryEntry>;
      { data_store.load_all_accounts() } ->
          std::same_as<std::vector<DirectoryEntry>>;
      { data_store.load_all_directories() } ->
          std::same_as<std::vector<DirectoryEntry>>;
      { data_store.load_account(std::declval<const std::string&>()) } ->
          std::same_as<DirectoryEntry>;
      { data_store.make_account(std::declval<const std::string&>(),
          std::declval<const std::string&>(),
          std::declval<const DirectoryEntry&>(),
          std::declval<boost::posix_time::ptime>()) } ->
            std::same_as<DirectoryEntry>;
      { data_store.make_directory(std::declval<const std::string&>(),
          std::declval<const DirectoryEntry&>()) } ->
            std::same_as<DirectoryEntry>;
      { data_store.remove(std::declval<const DirectoryEntry&>()) } ->
          std::same_as<void>;
      { data_store.associate(std::declval<const DirectoryEntry&>(),
          std::declval<const DirectoryEntry&>()) } -> std::same_as<bool>;
      { data_store.detach(std::declval<const DirectoryEntry&>(),
          std::declval<const DirectoryEntry&>()) } -> std::same_as<bool>;
      { data_store.load_password(std::declval<const DirectoryEntry&>()) } ->
          std::same_as<std::string>;
      { data_store.set_password(std::declval<const DirectoryEntry&>(),
          std::declval<const std::string&>()) } -> std::same_as<void>;
      { data_store.load_permissions(std::declval<const DirectoryEntry&>(),
          std::declval<const DirectoryEntry&>()) } -> std::same_as<Permissions>;
      { data_store.load_all_permissions(
          std::declval<const DirectoryEntry&>()) } ->
            std::same_as<std::vector<std::tuple<DirectoryEntry, Permissions>>>;
      { data_store.set_permissions(std::declval<const DirectoryEntry&>(),
          std::declval<const DirectoryEntry&>(),
          std::declval<Permissions>()) } -> std::same_as<void>;
      { data_store.load_registration_time(
          std::declval<const DirectoryEntry&>()) } ->
            std::same_as<boost::posix_time::ptime>;
      { data_store.load_last_login_time(
          std::declval<const DirectoryEntry&>()) } ->
            std::same_as<boost::posix_time::ptime>;
      { data_store.store_last_login_time(std::declval<const DirectoryEntry&>(),
          std::declval<boost::posix_time::ptime>()) } -> std::same_as<void>;
      { data_store.rename(std::declval<const DirectoryEntry&>(),
          std::declval<const std::string&>()) } -> std::same_as<void>;
      { data_store.with_transaction(std::declval<std::function<void ()>>()) };
    };

  /** Base class used to store ServiceLocatorClient data. */
  class ServiceLocatorDataStore {
    public:

      /**
       * Constructs a ServiceLocatorDataStore of a specified type using
       * emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsServiceLocatorDataStore T, typename... Args>
      explicit ServiceLocatorDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ServiceLocatorDataStore by referencing an existing data
       * store.
       * @param data_store The data store to reference.
       */
      template<DisableCopy<ServiceLocatorDataStore> T> requires
        IsServiceLocatorDataStore<dereference_t<T>>
      ServiceLocatorDataStore(T&& data_store);

      ServiceLocatorDataStore(const ServiceLocatorDataStore&) = default;

      /**
       * Loads all parents of a DirectoryEntry.
       * @param entry The DirectoryEntry whose parents are to be loaded.
       * @return All parents of the specified <i>entry</i>.
       */
      std::vector<DirectoryEntry> load_parents(const DirectoryEntry& entry);

      /**
       * Loads a directory's children.
       * @param directory The Directory whose children are to be returned.
       * @return The list of the <i>directory</i>'s children.
       */
      std::vector<DirectoryEntry> load_children(
        const DirectoryEntry& directory);

      /**
       * Loads a DirectoryEntry from its id.
       * @param id The id of the DirectoryEntry to load.
       * @return The DirectoryEntry with the specified <i>id</i>.
       */
      DirectoryEntry load_directory_entry(unsigned int id);

      /**
       * Loads all accounts.
       * @return The list of all DirectoryEntries representing accounts.
       */
      std::vector<DirectoryEntry> load_all_accounts();

      /**
       * Loads all directories.
       * @return The list of all DirectoryEntries representing directories.
       */
      std::vector<DirectoryEntry> load_all_directories();

      /**
       * Loads an account from its name.
       * @param name The name of the account.
       * @return The DirectoryEntry of the account with the specified
       *         <i>name</i>.
       */
      DirectoryEntry load_account(const std::string& name);

      /**
       * Creates an account.
       * @param name The name of the account.
       * @param password The account's password.
       * @param parent The initial parent to place the DirectoryEntry in.
       * @param registration_time The time of registration.
       * @return The DirectoryEntry of the account that was created.
       */
      DirectoryEntry make_account(const std::string& name,
        const std::string& password, const DirectoryEntry& parent,
        boost::posix_time::ptime registration_time);

      /**
       * Creates a directory.
       * @param name The name of the directory.
       * @param parent The initial parent to place the DirectoryEntry in.
       * @return The DirectoryEntry of the account that was created.
       */
      DirectoryEntry make_directory(
        const std::string& name, const DirectoryEntry& parent);

      /**
       * Deletes a DirectoryEntry.
       * @param entry The DirectoryEntry to delete.
       */
      void remove(const DirectoryEntry& entry);

      /**
       * Associates a DirectoryEntry with a parent.
       * @param entry The DirectoryEntry to associate.
       * @param parent The parent to associate the <i>entry</i> with.
       * @return <code>true</code> iff the <i>entry</i> was associated with
       *         <i>parent</i>.
       */
      bool associate(const DirectoryEntry& entry, const DirectoryEntry& parent);

      /**
       * Detaches a DirectoryEntry from one of its parents.
       * @param entry The DirectoryEntry to detach.
       * @param parent The parent to detach the <i>entry</i> from.
       * @return <code>true</code> iff the <i>entry</i> was detached from
       *         <i>parent</i>.
       */
      bool detach(const DirectoryEntry& entry, const DirectoryEntry& parent);

      /**
       * Loads an account's password.
       * @param account The account whose password is to be loaded.
       * @return The password associated with the <i>account</i>.
       */
      std::string load_password(const DirectoryEntry& account);

      /**
       * Sets an account's password.
       * @param account The account to set the password for.
       * @param password The new password.
       */
      void set_password(
        const DirectoryEntry& account, const std::string& password);

      /**
       * Loads the Permissions a DirectoryEntry has over another.
       * @param source The DirectoryEntry to check.
       * @param target The DirectoryEntry to check.
       * @return The <i>source</i>'s Permissions over the <i>target</i>.
       */
      Permissions load_permissions(
        const DirectoryEntry& source, const DirectoryEntry& target);

      /**
       * Loads all of the Permissions an account has.
       * @param account The DirectoryEntry to check.
       * @return The list of Permissions the <i>account</i> has.
       */
      std::vector<std::tuple<DirectoryEntry, Permissions>> load_all_permissions(
        const DirectoryEntry& account);

      /**
       * Sets the Permissions of one DirectoryEntry over another.
       * @param source The DirectoryEntry to grant permissions to.
       * @param target The DirectoryEntry to grant permissions over.
       * @param permissions The Permissions to grant the <i>source</i> over the
       *        <i>target</i>.
       */
      void set_permissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions);

      /**
       * Loads the registration time of an account.
       * @param account The account whose registration time is to be loaded.
       * @return The time of the <i>account</i>'s registration.
       */
      boost::posix_time::ptime load_registration_time(
        const DirectoryEntry& account);

      /**
       * Loads the time of an account's most recent login.
       * @param account The account that logged in.
       * @return The time of the <i>account</i>'s most recent login.
       */
      boost::posix_time::ptime load_last_login_time(
        const DirectoryEntry& account);

      /**
       * Stores the time of an account's most recent login.
       * @param account The account that logged in.
       * @param login_time The time of the <i>account</i>'s most recent login.
       */
      void store_last_login_time(
        const DirectoryEntry& account, boost::posix_time::ptime login_time);

      /**
       * Renames a DirectoryEntry.
       * @param entry The DirectoryEntry to rename.
       * @param name The name to assign to the <i>entry</i>.
       */
      void rename(const DirectoryEntry& entry, const std::string& name);

      /**
       * Performs an atomic transaction.
       * @param transaction The transaction to perform.
       */
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);

      void close();

    private:
      struct VirtualDataStore {
        virtual ~VirtualDataStore() = default;

        virtual std::vector<DirectoryEntry> load_parents(
          const DirectoryEntry&) = 0;
        virtual std::vector<DirectoryEntry> load_children(
          const DirectoryEntry&) = 0;
        virtual DirectoryEntry load_directory_entry(unsigned int) = 0;
        virtual std::vector<DirectoryEntry> load_all_accounts() = 0;
        virtual std::vector<DirectoryEntry> load_all_directories() = 0;
        virtual DirectoryEntry load_account(const std::string&) = 0;
        virtual DirectoryEntry make_account(const std::string&,
          const std::string&, const DirectoryEntry&,
          boost::posix_time::ptime) = 0;
        virtual DirectoryEntry make_directory(
          const std::string&, const DirectoryEntry&) = 0;
        virtual void remove(const DirectoryEntry&) = 0;
        virtual bool associate(
          const DirectoryEntry&, const DirectoryEntry&) = 0;
        virtual bool detach(const DirectoryEntry&, const DirectoryEntry&) = 0;
        virtual std::string load_password(const DirectoryEntry&) = 0;
        virtual void set_password(
          const DirectoryEntry&, const std::string&) = 0;
        virtual Permissions load_permissions(
          const DirectoryEntry&, const DirectoryEntry&) = 0;
        virtual std::vector<std::tuple<DirectoryEntry, Permissions>>
          load_all_permissions(const DirectoryEntry&) = 0;
        virtual void set_permissions(
          const DirectoryEntry&, const DirectoryEntry&, Permissions) = 0;
        virtual boost::posix_time::ptime load_registration_time(
          const DirectoryEntry&) = 0;
        virtual boost::posix_time::ptime load_last_login_time(
          const DirectoryEntry&) = 0;
        virtual void store_last_login_time(
          const DirectoryEntry&, boost::posix_time::ptime) = 0;
        virtual void rename(const DirectoryEntry&, const std::string&) = 0;
        virtual void with_transaction(const std::function<void ()>&) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedDataStore final : VirtualDataStore {
        using DataStore = D;
        local_ptr_t<DataStore> m_data_store;

        template<typename... Args>
        WrappedDataStore(Args&&... args);

        std::vector<DirectoryEntry> load_parents(
          const DirectoryEntry& entry) override;
        std::vector<DirectoryEntry> load_children(
          const DirectoryEntry& directory) override;
        DirectoryEntry load_directory_entry(unsigned int id) override;
        std::vector<DirectoryEntry> load_all_accounts() override;
        std::vector<DirectoryEntry> load_all_directories() override;
        DirectoryEntry load_account(const std::string& name) override;
        DirectoryEntry make_account(const std::string& name,
          const std::string& password, const DirectoryEntry& parent,
          boost::posix_time::ptime registration_time) override;
        DirectoryEntry make_directory(
          const std::string& name, const DirectoryEntry& parent) override;
        void remove(const DirectoryEntry& entry) override;
        bool associate(
          const DirectoryEntry& entry, const DirectoryEntry& parent) override;
        bool detach(
          const DirectoryEntry& entry, const DirectoryEntry& parent) override;
        std::string load_password(const DirectoryEntry& account) override;
        void set_password(
          const DirectoryEntry& account, const std::string& password) override;
        Permissions load_permissions(
          const DirectoryEntry& source, const DirectoryEntry& target) override;
        std::vector<std::tuple<DirectoryEntry, Permissions>>
          load_all_permissions(const DirectoryEntry& account) override;
        void set_permissions(const DirectoryEntry& source,
          const DirectoryEntry& target, Permissions permissions) override;
        boost::posix_time::ptime load_registration_time(
          const DirectoryEntry& account) override;
        boost::posix_time::ptime load_last_login_time(
          const DirectoryEntry& account) override;
        void store_last_login_time(const DirectoryEntry& account,
          boost::posix_time::ptime login_time) override;
        void rename(
          const DirectoryEntry& entry, const std::string& name) override;
        void with_transaction(
          const std::function<void ()>& transaction) override;
        void close() override;
      };
      VirtualPtr<VirtualDataStore> m_data_store;
  };

  /**
   * Returns <code>true</code> if a DirectoryEntry has a given permission.
   * @param data_store The ServiceLocatorDataStore storing the permissions.
   * @param source The DirectoryEntry to check.
   * @param target The DirectoryEntry to check.
   * @param permissions The permissions to test for.
   * @return <code>true</code> iff the <i>source</i> has the specified
   *         <i>permissions</i> over the <i>target</i>.
   */
  template<IsServiceLocatorDataStore D>
  bool has_permission(D& data_store, const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    struct HasPermissionHelper {
      bool operator()(D& data_store, const DirectoryEntry& source,
          const DirectoryEntry& target, Permissions permissions,
          std::unordered_set<DirectoryEntry>& visited_entries) const {
        if(!visited_entries.insert(target).second) {
          return false;
        }
        auto account_permissions = data_store.load_permissions(source, target);
        if((account_permissions & permissions) == permissions) {
          return true;
        }
        auto parents = data_store.load_parents(target);
        if(parents.empty()) {
          return false;
        }
        for(auto& parent : parents) {
          if(HasPermissionHelper()(data_store, source, parent, permissions,
              visited_entries)) {
            return true;
          }
        }
        return false;
      }
    };
    if(source == target &&
        ((permissions & Permissions(Permission::READ)) == permissions)) {
      return true;
    }
    auto visited_entries = std::unordered_set<DirectoryEntry>();
    return HasPermissionHelper()(
      data_store, source, target, permissions, visited_entries);
  }

  /**
   * Returns the DirectoryEntry at a specified path.
   * @param data_store The ServiceLocatorDataStore to search for the path.
   * @param root The root DirectoryEntry to begin searching from.
   * @param path The path to search for.
   * @return The DirectoryEntry at the specified <i>path</i>.
   */
  template<IsServiceLocatorDataStore D>
  DirectoryEntry load_directory_entry(D& data_store, const DirectoryEntry& root,
      const std::string& path) {
    if(path.empty()) {
      boost::throw_with_location(
        ServiceLocatorDataStoreException("Directory entry not found."));
    }
    auto delimiter = path.find('/');
    auto segment = std::string();
    if(delimiter == std::string::npos) {
      segment = path;
    } else {
      segment = path.substr(0, delimiter);
    }
    auto children = data_store.load_children(root);
    for(auto& child : children) {
      if(child.m_name == segment) {
        if(delimiter == std::string::npos) {
          return child;
        } else {
          return load_directory_entry(
            data_store, child, path.substr(delimiter + 1));
        }
      }
    }
    boost::throw_with_location(
      ServiceLocatorDataStoreException("Directory entry not found."));
  }

  template<IsServiceLocatorDataStore D>
  DirectoryEntry validate(D& data_store, const DirectoryEntry& entry) {
    auto validated_entry = data_store.load_directory_entry(entry.m_id);
    if(validated_entry != entry) {
      boost::throw_with_location(ServiceLocatorDataStoreException(
        "Directory entry not found."));
    }
    return validated_entry;
  }

  /**
   * Returns a hashed password from a DirectoryEntry and a plain-text password.
   * @param account The account to make the password for.
   * @param password The plain-text password to hash.
   * @return A hashed password for the <i>account</i>.
   */
  inline std::string hash_password(
      const DirectoryEntry& account, std::string_view password) {
    return bcrypt(password);
  }

  /**
   * Validates a password.
   * @param account The account to validate the password for.
   * @param received_password The password received from the client.
   * @param stored_password The password stored for the <i>account</i>.
   * @return <code>true</code> iff the <i>received_password<i> matches the
   *         <i>stored_password</i>.
   */
  inline bool validate_password(
      const DirectoryEntry& account, std::string_view received_password,
      std::string_view stored_password) {
    if(!stored_password.empty() && stored_password[0] == '$') {
      return bcrypt_matches(received_password, stored_password);
    }
    auto salted_password = std::to_string(account.m_id);
    salted_password.reserve(
      salted_password.size() + received_password.size());
    salted_password.append(received_password.data(), received_password.size());
    auto received_password_hash = compute_sha(salted_password);
    return received_password_hash == stored_password;
  }

  template<IsServiceLocatorDataStore T, typename... Args>
  ServiceLocatorDataStore::ServiceLocatorDataStore(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(
        make_virtual_ptr<WrappedDataStore<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<ServiceLocatorDataStore> T> requires
    IsServiceLocatorDataStore<dereference_t<T>>
  ServiceLocatorDataStore::ServiceLocatorDataStore(T&& data_store)
    : m_data_store(make_virtual_ptr<WrappedDataStore<std::remove_cvref_t<T>>>(
        std::forward<T>(data_store))) {}

  inline std::vector<DirectoryEntry> ServiceLocatorDataStore::load_parents(
      const DirectoryEntry& entry) {
    return m_data_store->load_parents(entry);
  }

  inline std::vector<DirectoryEntry> ServiceLocatorDataStore::load_children(
      const DirectoryEntry& directory) {
    return m_data_store->load_children(directory);
  }

  inline DirectoryEntry ServiceLocatorDataStore::load_directory_entry(
      unsigned int id) {
    return m_data_store->load_directory_entry(id);
  }

  inline std::vector<DirectoryEntry>
      ServiceLocatorDataStore::load_all_accounts() {
    return m_data_store->load_all_accounts();
  }

  inline std::vector<DirectoryEntry>
      ServiceLocatorDataStore::load_all_directories() {
    return m_data_store->load_all_directories();
  }

  inline DirectoryEntry ServiceLocatorDataStore::load_account(
      const std::string& name) {
    return m_data_store->load_account(name);
  }

  inline DirectoryEntry ServiceLocatorDataStore::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      boost::posix_time::ptime registration_time) {
    return m_data_store->make_account(
      name, password, parent, registration_time);
  }

  inline DirectoryEntry ServiceLocatorDataStore::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    return m_data_store->make_directory(name, parent);
  }

  inline void ServiceLocatorDataStore::remove(const DirectoryEntry& entry) {
    m_data_store->remove(entry);
  }

  inline bool ServiceLocatorDataStore::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    return m_data_store->associate(entry, parent);
  }

  inline bool ServiceLocatorDataStore::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    return m_data_store->detach(entry, parent);
  }

  inline std::string ServiceLocatorDataStore::load_password(
      const DirectoryEntry& account) {
    return m_data_store->load_password(account);
  }

  inline void ServiceLocatorDataStore::set_password(
      const DirectoryEntry& account, const std::string& password) {
    m_data_store->set_password(account, password);
  }

  inline Permissions ServiceLocatorDataStore::load_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    return m_data_store->load_permissions(source, target);
  }

  inline std::vector<std::tuple<DirectoryEntry, Permissions>>
      ServiceLocatorDataStore::load_all_permissions(
        const DirectoryEntry& account) {
    return m_data_store->load_all_permissions(account);
  }

  inline void ServiceLocatorDataStore::set_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_data_store->set_permissions(source, target, permissions);
  }

  inline boost::posix_time::ptime
      ServiceLocatorDataStore::load_registration_time(
        const DirectoryEntry& account) {
    return m_data_store->load_registration_time(account);
  }

  inline boost::posix_time::ptime ServiceLocatorDataStore::load_last_login_time(
      const DirectoryEntry& account) {
    return m_data_store->load_last_login_time(account);
  }

  inline void ServiceLocatorDataStore::store_last_login_time(
      const DirectoryEntry& account, boost::posix_time::ptime login_time) {
    m_data_store->store_last_login_time(account, login_time);
  }

  inline void ServiceLocatorDataStore::rename(
      const DirectoryEntry& entry, const std::string& name) {
    m_data_store->rename(entry, name);
  }

  template<std::invocable<> F>
  decltype(auto) ServiceLocatorDataStore::with_transaction(F&& transaction) {
    using R = std::invoke_result_t<F>;
    if constexpr(std::is_reference_v<R>) {
      auto result = static_cast<std::remove_reference_t<R>*>(nullptr);
      m_data_store->with_transaction([&] {
        result = &(std::forward<F>(transaction)());
      });
      return *result;
    } else if constexpr(std::is_void_v<R>) {
      m_data_store->with_transaction(std::forward<F>(transaction));
    } else {
      auto result = boost::optional<R>();
      m_data_store->with_transaction([&] {
        result.emplace(std::forward<F>(transaction)());
      });
      return R(std::move(*result));
    }
  }

  inline void ServiceLocatorDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  ServiceLocatorDataStore::WrappedDataStore<D>::WrappedDataStore(Args&&... args)
    : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  std::vector<DirectoryEntry>
      ServiceLocatorDataStore::WrappedDataStore<D>::load_parents(
        const DirectoryEntry& entry) {
    return m_data_store->load_parents(entry);
  }

  template<typename D>
  std::vector<DirectoryEntry>
      ServiceLocatorDataStore::WrappedDataStore<D>::load_children(
        const DirectoryEntry& directory) {
    return m_data_store->load_children(directory);
  }

  template<typename D>
  DirectoryEntry ServiceLocatorDataStore::WrappedDataStore<D>::
      load_directory_entry(unsigned int id) {
    return m_data_store->load_directory_entry(id);
  }

  template<typename D>
  std::vector<DirectoryEntry>
      ServiceLocatorDataStore::WrappedDataStore<D>::load_all_accounts() {
    return m_data_store->load_all_accounts();
  }

  template<typename D>
  std::vector<DirectoryEntry>
      ServiceLocatorDataStore::WrappedDataStore<D>::load_all_directories() {
    return m_data_store->load_all_directories();
  }

  template<typename D>
  DirectoryEntry ServiceLocatorDataStore::WrappedDataStore<D>::load_account(
      const std::string& name) {
    return m_data_store->load_account(name);
  }

  template<typename D>
  DirectoryEntry ServiceLocatorDataStore::WrappedDataStore<D>::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      boost::posix_time::ptime registration_time) {
    return m_data_store->make_account(
      name, password, parent, registration_time);
  }

  template<typename D>
  DirectoryEntry ServiceLocatorDataStore::WrappedDataStore<D>::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    return m_data_store->make_directory(name, parent);
  }

  template<typename D>
  void ServiceLocatorDataStore::WrappedDataStore<D>::remove(
      const DirectoryEntry& entry) {
    m_data_store->remove(entry);
  }

  template<typename D>
  bool ServiceLocatorDataStore::WrappedDataStore<D>::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    return m_data_store->associate(entry, parent);
  }

  template<typename D>
  bool ServiceLocatorDataStore::WrappedDataStore<D>::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    return m_data_store->detach(entry, parent);
  }

  template<typename D>
  std::string ServiceLocatorDataStore::WrappedDataStore<D>::load_password(
      const DirectoryEntry& account) {
    return m_data_store->load_password(account);
  }

  template<typename D>
  void ServiceLocatorDataStore::WrappedDataStore<D>::set_password(
      const DirectoryEntry& account, const std::string& password) {
    m_data_store->set_password(account, password);
  }

  template<typename D>
  Permissions ServiceLocatorDataStore::WrappedDataStore<D>::load_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    return m_data_store->load_permissions(source, target);
  }

  template<typename D>
  std::vector<std::tuple<DirectoryEntry, Permissions>>
      ServiceLocatorDataStore::WrappedDataStore<D>::load_all_permissions(
        const DirectoryEntry& account) {
    return m_data_store->load_all_permissions(account);
  }

  template<typename D>
  void ServiceLocatorDataStore::WrappedDataStore<D>::set_permissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    m_data_store->set_permissions(source, target, permissions);
  }

  template<typename D>
  boost::posix_time::ptime
      ServiceLocatorDataStore::WrappedDataStore<D>::load_registration_time(
        const DirectoryEntry& account) {
    return m_data_store->load_registration_time(account);
  }

  template<typename D>
  boost::posix_time::ptime
      ServiceLocatorDataStore::WrappedDataStore<D>::load_last_login_time(
        const DirectoryEntry& account) {
    return m_data_store->load_last_login_time(account);
  }

  template<typename D>
  void ServiceLocatorDataStore::WrappedDataStore<D>::store_last_login_time(
      const DirectoryEntry& account, boost::posix_time::ptime login_time) {
    m_data_store->store_last_login_time(account, login_time);
  }

  template<typename D>
  void ServiceLocatorDataStore::WrappedDataStore<D>::rename(
      const DirectoryEntry& entry, const std::string& name) {
    m_data_store->rename(entry, name);
  }

  template<typename D>
  void ServiceLocatorDataStore::WrappedDataStore<D>::with_transaction(
      const std::function<void ()>& transaction) {
    m_data_store->with_transaction(transaction);
  }

  template<typename D>
  void ServiceLocatorDataStore::WrappedDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
