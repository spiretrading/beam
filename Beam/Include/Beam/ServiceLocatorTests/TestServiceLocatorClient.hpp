#ifndef BEAM_TEST_SERVICE_LOCATOR_CLIENT_HPP
#define BEAM_TEST_SERVICE_LOCATOR_CLIENT_HPP
#include <variant>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>
#include "Beam/Queues/Queue.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/ServicesTests/ServiceResult.hpp"
#include "Beam/ServicesTests/TestServiceClientOperationQueue.hpp"

namespace Beam::Tests {

  /**
   * Implements a ServiceLocatorClient for testing purposes by pushing all
   * operations performed on this client onto a queue.
   */
  class TestServiceLocatorClient {
    public:

      /** Records a call to authenticate_account(...). */
      struct AuthenticateAccountOperation {

        /** The username passed. */
        std::string m_username;

        /** The password passed. */
        std::string m_password;

        /** Used to return a value to the caller. */
        ServiceResult<DirectoryEntry> m_result;
      };

      /** Records a call to authenticate_session(...). */
      struct AuthenticateSessionOperation {

        /** The session id passed. */
        std::string m_session_id;

        /** The encryption key passed. */
        unsigned int m_key;

        /** Used to return a value to the caller. */
        ServiceResult<DirectoryEntry> m_result;
      };

      /** Records a call to locate(...). */
      struct LocateOperation {

        /** The service name passed. */
        std::string m_name;

        /** Used to return a value to the caller. */
        ServiceResult<std::vector<ServiceEntry>> m_result;
      };

      /** Records a call to add(...). */
      struct AddOperation {

        /** The service name passed. */
        std::string m_name;

        /** The service properties passed. */
        JsonObject m_properties;

        /** Used to return a value to the caller. */
        ServiceResult<ServiceEntry> m_result;
      };

      /** Records a call to remove(const ServiceEntry&). */
      struct RemoveServiceOperation {

        /** The service passed. */
        ServiceEntry m_service;

        /** Used to return a value to the caller. */
        ServiceResult<void> m_result;
      };

      /** Records a call to load_all_accounts(). */
      struct LoadAllAccountsOperation {

        /** Used to return a value to the caller. */
        ServiceResult<std::vector<DirectoryEntry>> m_result;
      };

      /** Records a call to find_account(...). */
      struct FindAccountOperation {

        /** The account name passed. */
        std::string m_name;

        /** Used to return a value to the caller. */
        ServiceResult<boost::optional<DirectoryEntry>> m_result;
      };

      /** Records a call to make_account(...). */
      struct MakeAccountOperation {

        /** The account name passed. */
        std::string m_name;

        /** The password passed. */
        std::string m_password;

        /** The parent directory entry passed. */
        DirectoryEntry m_parent;

        /** Used to return a value to the caller. */
        ServiceResult<DirectoryEntry> m_result;
      };

      /** Records a call to make_directory(...). */
      struct MakeDirectoryOperation {

        /** The directory name passed. */
        std::string m_name;

        /** The parent directory entry passed. */
        DirectoryEntry m_parent;

        /** Used to return a value to the caller. */
        ServiceResult<DirectoryEntry> m_result;
      };

      /** Records a call to store_password(...). */
      struct StorePasswordOperation {

        /** The account passed. */
        DirectoryEntry m_account;

        /** The password passed. */
        std::string m_password;

        /** Used to return a value to the caller. */
        ServiceResult<void> m_result;
      };

      /** Records a call to monitor(...). */
      struct MonitorOperation {

        /** The queue writer for AccountUpdate. */
        ScopedQueueWriter<AccountUpdate> m_queue;
      };

      /** Records a call to load_directory_entry. */
      struct LoadDirectoryEntryByPathOperation {

        /** The root directory entry passed. */
        DirectoryEntry m_root;

        /** The path passed. */
        std::string m_path;

        /** Used to return a value to the caller. */
        ServiceResult<DirectoryEntry> m_result;
      };

      /** Records a call to load_directory_entry. */
      struct LoadDirectoryEntryByIdOperation {

        /** The id passed. */
        unsigned int m_id;

        /** Used to return a value to the caller. */
        ServiceResult<DirectoryEntry> m_result;
      };

      /** Records a call to load_parents(...). */
      struct LoadParentsOperation {

        /** The directory entry passed. */
        DirectoryEntry m_entry;

        /** Used to return a value to the caller. */
        ServiceResult<std::vector<DirectoryEntry>> m_result;
      };

      /** Records a call to load_children(...). */
      struct LoadChildrenOperation {

        /** The directory entry passed. */
        DirectoryEntry m_entry;

        /** Used to return a value to the caller. */
        ServiceResult<std::vector<DirectoryEntry>> m_result;
      };

      /** Records a call to remove. */
      struct RemoveDirectoryEntryOperation {

        /** The directory entry passed. */
        DirectoryEntry m_entry;

        /** Used to return a value to the caller. */
        ServiceResult<void> m_result;
      };

      /** Records a call to associate(...). */
      struct AssociateOperation {

        /** The directory entry passed. */
        DirectoryEntry m_entry;

        /** The parent directory entry passed. */
        DirectoryEntry m_parent;

        /** Used to return a value to the caller. */
        ServiceResult<void> m_result;
      };

      /** Records a call to detach(...). */
      struct DetachOperation {

        /** The directory entry passed. */
        DirectoryEntry m_entry;

        /** The parent directory entry passed. */
        DirectoryEntry m_parent;

        /** Used to return a value to the caller. */
        ServiceResult<void> m_result;
      };

      /** Records a call to has_permissions(...). */
      struct HasPermissionsOperation {

        /** The account passed. */
        DirectoryEntry m_account;

        /** The target directory entry passed. */
        DirectoryEntry m_target;

        /** The permissions passed. */
        Permissions m_permissions;

        /** Used to return a value to the caller. */
        ServiceResult<bool> m_result;
      };

      /** Records a call to store(...). */
      struct StorePermissionsOperation {

        /** The source directory entry passed. */
        DirectoryEntry m_source;

        /** The target directory entry passed. */
        DirectoryEntry m_target;

        /** The permissions passed. */
        Permissions m_permissions;

        /** Used to return a value to the caller. */
        ServiceResult<void> m_result;
      };

      /** Records a call to load_registration_time(...). */
      struct LoadRegistrationTimeOperation {

        /** The account passed. */
        DirectoryEntry m_account;

        /** Used to return a value to the caller. */
        ServiceResult<boost::posix_time::ptime> m_result;
      };

      /** Records a call to load_last_login_time(...). */
      struct LoadLastLoginTimeOperation {

        /** The account passed. */
        DirectoryEntry m_account;

        /** Used to return a value to the caller. */
        ServiceResult<boost::posix_time::ptime> m_result;
      };

      /** Records a call to rename(...). */
      struct RenameOperation {

        /** The directory entry passed. */
        DirectoryEntry m_entry;

        /** The new name passed. */
        std::string m_name;

        /** Used to return a value to the caller. */
        ServiceResult<DirectoryEntry> m_result;
      };

      /** A variant covering all possible TestServiceLocatorClient operations. */
      using Operation = std::variant<AuthenticateAccountOperation,
        AuthenticateSessionOperation, LocateOperation, AddOperation,
        RemoveServiceOperation, LoadAllAccountsOperation, FindAccountOperation,
        MakeAccountOperation, MakeDirectoryOperation, StorePasswordOperation,
        MonitorOperation, LoadDirectoryEntryByPathOperation,
        LoadDirectoryEntryByIdOperation, LoadParentsOperation,
        LoadChildrenOperation, RemoveDirectoryEntryOperation,
        AssociateOperation, DetachOperation, HasPermissionsOperation,
        StorePermissionsOperation, LoadRegistrationTimeOperation,
        LoadLastLoginTimeOperation, RenameOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestServiceLocatorClient.
       * @param account The account that this client represents.
       * @param session_id The session id for this client.
       * @param operations The queue to push all operations on.
       */
      explicit TestServiceLocatorClient(
        DirectoryEntry account, std::string session_id,
        ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept;

      ~TestServiceLocatorClient();

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
      DirectoryEntry m_account;
      std::string m_session_id;
      TestServiceClientOperationQueue<Operation> m_operations;

      TestServiceLocatorClient(const TestServiceLocatorClient&) = delete;
      TestServiceLocatorClient& operator =(
        const TestServiceLocatorClient&) = delete;
  };

  inline TestServiceLocatorClient::TestServiceLocatorClient(
    DirectoryEntry account, std::string session_id,
    ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_account(std::move(account)),
      m_session_id(std::move(session_id)),
      m_operations(std::move(operations)) {}

  inline TestServiceLocatorClient::~TestServiceLocatorClient() {
    close();
  }

  inline DirectoryEntry TestServiceLocatorClient::get_account() const {
    return m_account;
  }

  inline std::string TestServiceLocatorClient::get_session_id() const {
    return m_session_id;
  }

  inline std::string TestServiceLocatorClient::get_encrypted_session_id(
      unsigned int key) const {
    return compute_sha(std::to_string(key) + m_session_id);
  }

  inline DirectoryEntry TestServiceLocatorClient::authenticate_account(
      const std::string& username, const std::string& password) {
    return m_operations.append_result<
      AuthenticateAccountOperation, DirectoryEntry>(username, password);
  }

  inline DirectoryEntry TestServiceLocatorClient::authenticate_session(
      const std::string& session_id, unsigned int key) {
    return m_operations.append_result<
      AuthenticateSessionOperation, DirectoryEntry>(session_id, key);
  }

  inline std::vector<ServiceEntry> TestServiceLocatorClient::locate(
      const std::string& name) {
    return m_operations.append_result<
      LocateOperation, std::vector<ServiceEntry>>(name);
  }

  inline ServiceEntry TestServiceLocatorClient::add(
      const std::string& name, const JsonObject& properties) {
    return m_operations.append_result<AddOperation, ServiceEntry>(
      name, properties);
  }

  inline void TestServiceLocatorClient::remove(const ServiceEntry& service) {
    m_operations.append_result<RemoveServiceOperation, void>(service);
  }

  inline std::vector<DirectoryEntry>
      TestServiceLocatorClient::load_all_accounts() {
    return m_operations.append_result<
      LoadAllAccountsOperation, std::vector<DirectoryEntry>>();
  }

  inline boost::optional<DirectoryEntry> TestServiceLocatorClient::find_account(
      const std::string& name) {
    return m_operations.append_result<
      FindAccountOperation, boost::optional<DirectoryEntry>>(name);
  }

  inline DirectoryEntry TestServiceLocatorClient::make_account(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent) {
    return m_operations.append_result<
      MakeAccountOperation, DirectoryEntry>(name, password, parent);
  }

  inline DirectoryEntry TestServiceLocatorClient::make_directory(
      const std::string& name, const DirectoryEntry& parent) {
    return m_operations.append_result<
      MakeDirectoryOperation, DirectoryEntry>(name, parent);
  }

  inline void TestServiceLocatorClient::store_password(
      const DirectoryEntry& account, const std::string& password) {
    m_operations.append_result<StorePasswordOperation, void>(
      account, password);
  }

  inline void TestServiceLocatorClient::monitor(
      ScopedQueueWriter<AccountUpdate> queue) {
    m_operations.append_queue<MonitorOperation>(std::make_shared<Operation>(
      MonitorOperation(std::move(queue))));
  }

  inline DirectoryEntry TestServiceLocatorClient::load_directory_entry(
      const DirectoryEntry& root, const std::string& path) {
    return m_operations.append_result<
      LoadDirectoryEntryByPathOperation, DirectoryEntry>(root, path);
  }

  inline DirectoryEntry TestServiceLocatorClient::load_directory_entry(
      unsigned int id) {
    return m_operations.append_result<
      LoadDirectoryEntryByIdOperation, DirectoryEntry>(id);
  }

  inline std::vector<DirectoryEntry> TestServiceLocatorClient::load_parents(
      const DirectoryEntry& entry) {
    return m_operations.append_result<
      LoadParentsOperation, std::vector<DirectoryEntry>>(entry);
  }

  inline std::vector<DirectoryEntry> TestServiceLocatorClient::load_children(
      const DirectoryEntry& entry) {
    return m_operations.append_result<
      LoadChildrenOperation, std::vector<DirectoryEntry>>(entry);
  }

  inline void TestServiceLocatorClient::remove(const DirectoryEntry& entry) {
    m_operations.append_result<RemoveDirectoryEntryOperation, void>(entry);
  }

  inline void TestServiceLocatorClient::associate(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_operations.append_result<AssociateOperation, void>(entry, parent);
  }

  inline void TestServiceLocatorClient::detach(
      const DirectoryEntry& entry, const DirectoryEntry& parent) {
    m_operations.append_result<DetachOperation, void>(entry, parent);
  }

  inline bool TestServiceLocatorClient::has_permissions(
      const DirectoryEntry& account, const DirectoryEntry& target,
      Permissions permissions) {
    return m_operations.append_result<HasPermissionsOperation, bool>(
      account, target, permissions);
  }

  inline void TestServiceLocatorClient::store(const DirectoryEntry& source,
      const DirectoryEntry& target, Permissions permissions) {
    m_operations.append_result<StorePermissionsOperation, void>(
      source, target, permissions);
  }

  inline boost::posix_time::ptime
      TestServiceLocatorClient::load_registration_time(
        const DirectoryEntry& account) {
    return m_operations.append_result<
      LoadRegistrationTimeOperation, boost::posix_time::ptime>(account);
  }

  inline boost::posix_time::ptime
      TestServiceLocatorClient::load_last_login_time(
        const DirectoryEntry& account) {
    return m_operations.append_result<
      LoadLastLoginTimeOperation, boost::posix_time::ptime>(account);
  }

  inline DirectoryEntry TestServiceLocatorClient::rename(
      const DirectoryEntry& entry, const std::string& name) {
    return m_operations.append_result<RenameOperation, DirectoryEntry>(
      entry, name);
  }

  inline void TestServiceLocatorClient::close() {
    m_operations.close();
  }
}

#endif
