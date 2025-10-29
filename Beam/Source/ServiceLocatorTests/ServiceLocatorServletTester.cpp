#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::ServiceLocatorServices;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Fixture {
    using TestServiceProtocolServletContainer = ServiceProtocolServletContainer<
      MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>,
      LocalServerConnection*, BinarySender<SharedBuffer>, NullEncoder,
      std::shared_ptr<TriggerTimer>>;
    using ClientServiceProtocolClient = ServiceProtocolClient<MessageProtocol<
      LocalClientChannel, BinarySender<SharedBuffer>, NullEncoder>,
      TriggerTimer>;
    LocalServiceLocatorDataStore m_data_store;
    LocalServerConnection m_server_connection;
    TestServiceProtocolServletContainer m_container;
    optional<ClientServiceProtocolClient> m_client_protocol;

    Fixture()
        : m_container(init(&m_data_store), &m_server_connection,
            factory<std::shared_ptr<TriggerTimer>>()) {
      m_client_protocol.emplace(init("test", m_server_connection), init());
      register_service_locator_services(out(m_client_protocol->get_slots()));
      register_service_locator_messages(out(m_client_protocol->get_slots()));
    }

    DirectoryEntry create_user(
        const std::string& username, const std::string& password) {
      auto account = m_data_store.make_account(username, password,
        DirectoryEntry::STAR_DIRECTORY, second_clock::universal_time());
      REQUIRE(account.m_id != -1);
      return account;
    }

    void create_account_and_login(
        Out<DirectoryEntry> account, Out<std::string> session_id) {
      auto username = std::string("user1");
      auto password = std::string("password");
      create_user(username, password);
      auto result =
        m_client_protocol->send_request<LoginService>(username, password);
      *account = result.account;
      *session_id = result.session_id;
    }

    void create_additional_client(const std::string& username,
        const std::string& password, Out<DirectoryEntry> account,
        Out<std::string> session_id,
        Out<std::optional<ClientServiceProtocolClient>> service) {
      service->emplace(init("test", m_server_connection), init());
      register_service_locator_services(out((*service)->get_slots()));
      create_user(username, password);
      auto result = (*service)->send_request<LoginService>(username, password);
      *account = result.account;
      *session_id = result.session_id;
    }
  };
}

TEST_SUITE("ServiceLocatorServlet") {
  TEST_CASE_FIXTURE(Fixture, "login_with_invalid_account") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoginService>(
      "invalid_username", ""), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "login_with_invalid_password") {
    auto username = std::string("user1");
    auto password = std::string("password");
    create_user(username, password);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoginService>(username,
      "1234"), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_login") {
    auto username = std::string("user1");
    auto password = std::string("password");
    create_user(username, password);
    auto loginResult = m_client_protocol->send_request<LoginService>(username,
      password);
    REQUIRE(loginResult.account.m_name == username);
    REQUIRE(loginResult.session_id.size() == SESSION_ID_LENGTH);

    // Test logging in again, duplicate login.
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoginService>(username,
      password), ServiceRequestException);
  }

#if 0
  TEST_CASE_FIXTURE(Fixture, "authenticate_account_without_login") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      AuthenticateAccountService>("invalid_user", "password"),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_with_invalid_username") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto loginResult = m_client_protocol->send_request<LoginService>(
      username, password);
    auto result = m_client_protocol->send_request<AuthenticateAccountService>(
      "invalid_user", "password");
    REQUIRE((result.m_type == DirectoryEntry::Type::NONE));
    REQUIRE(result.m_id == -1);
    REQUIRE(result.m_name.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_with_invalid_password") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto loginResult = m_client_protocol->send_request<LoginService>(
      username, password);
    auto result = m_client_protocol->send_request<AuthenticateAccountService>(
      "user1", "invalid_password");
    REQUIRE((result.m_type == DirectoryEntry::Type::NONE));
    REQUIRE(result.m_id == -1);
    REQUIRE(result.m_name.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_without_permission") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    auto loginResult = m_client_protocol->send_request<LoginService>(
      username, password);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      AuthenticateAccountService>("user1", "password"),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_authenticate_account") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto loginResult = m_client_protocol->send_request<LoginService>(
      username, password);
    auto result = m_client_protocol->send_request<AuthenticateAccountService>(
      "user1", "password");
    REQUIRE((result.m_type == DirectoryEntry::Type::ACCOUNT));
    REQUIRE(result.m_id == loginResult.account.m_id);
    REQUIRE(result.m_name == loginResult.account.m_name);
  }

  TEST_CASE_FIXTURE(Fixture, "session_authentication_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      SessionAuthenticationService>("", 0), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "session_authentication_with_invalid_session_id") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      SessionAuthenticationService>("", 0), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_session_authentication") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto key = 0U;
    auto encodedSessionId = ComputeSHA("0" + session_id);
    auto result = m_client_protocol->send_request<SessionAuthenticationService>(
      encodedSessionId, key);
    REQUIRE(result == account);
  }

  TEST_CASE_FIXTURE(Fixture, "register_service_without_login") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    REQUIRE_THROWS_AS(m_client_protocol->send_request<RegisterService>(service,
      properties), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "register_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto result = m_client_protocol->send_request<RegisterService>(service,
      properties);
    REQUIRE(result.GetName() == service);
    REQUIRE(result.GetProperties() == properties);
  }

  TEST_CASE_FIXTURE(Fixture, "unregister_service_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<UnregisterService>(1),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "unregister_invalid_service") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<UnregisterService>(1),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "register_unregister_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
  
    // Login and register the service.
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto registerResult = m_client_protocol->send_request<RegisterService>(
      service, properties);
    REQUIRE(registerResult.GetName() == service);
    REQUIRE(registerResult.GetProperties() == properties);
  
    // Unregister the service.
    m_client_protocol->send_request<UnregisterService>(registerResult.GetId());
  
    // Re-register the service.
    auto reregisterResult = m_client_protocol->send_request<RegisterService>(
      service, properties);
    REQUIRE(reregisterResult.GetName() == service);
    REQUIRE(reregisterResult.GetProperties() == properties);
    REQUIRE(reregisterResult.GetId() != registerResult.GetId());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LocateService>(""),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "locating_non_existing_service") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto result = m_client_protocol->send_request<LocateService>("");
    REQUIRE(result.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_single_provider_service") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto providerService = std::optional<ClientServiceProtocolClient>();
    auto provider = DirectoryEntry();
    auto providerSession = std::string();
    create_additional_client("user2", "password", Store(provider),
      Store(providerSession), Store(providerService));
  
    // Register the service using the mock provider.
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto registerResult = providerService->send_request<RegisterService>(
      "service", properties);
  
    // Locate the service using the mock client.
    auto locateResult = m_client_protocol->send_request<LocateService>("service");
    REQUIRE(locateResult.size() == 1);
    auto& replyService = locateResult.front();
    REQUIRE(replyService.GetName() == "service");
    REQUIRE(replyService.GetAccount().m_name == provider.m_name);
    REQUIRE(replyService.GetAccount().m_id == provider.m_id);
  
    // Unregister the service.
    providerService->send_request<UnregisterService>(replyService.GetId());
  
    // Redo the locate.
    auto relocateResult = m_client_protocol->send_request<LocateService>(
      "service");
    REQUIRE(relocateResult.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_multiple_provider_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto providerService = std::optional<ClientServiceProtocolClient>();
    auto firstProvider = DirectoryEntry();
    auto firstProviderSession = std::string();
    create_additional_client("user2", "password", Store(firstProvider),
      Store(firstProviderSession), Store(providerService));
  
    // Register the service using the first provider.
    auto firstServiceId = 1;
    auto secondServiceId = 2;
    auto registerResult = providerService->send_request<RegisterService>(service,
      properties);
    REQUIRE(registerResult.GetName() == service);
    REQUIRE(registerResult.GetProperties() == properties);
  
    // Register the service using the second provider.
    auto secondaryProviderService =
      std::optional<ClientServiceProtocolClient>();
    auto secondProvider = DirectoryEntry();
    auto secondProviderSession = std::string();
    create_additional_client("user3", "password", Store(secondProvider),
      Store(secondProviderSession), Store(secondaryProviderService));
    auto secondRegisterResult =
      secondaryProviderService->send_request<RegisterService>(service,
      properties);
    REQUIRE(secondRegisterResult.GetName() == service);
    REQUIRE(secondRegisterResult.GetProperties() == properties);
  
    // Locate the service using the mock client.
    auto services = m_client_protocol->send_request<LocateService>("service");
    REQUIRE(services.size() == 2);
    REQUIRE(services[0].GetAccount().m_id != services[1].GetAccount().m_id);
    auto replyService = services[0];
    REQUIRE(replyService.GetName() == service);
    REQUIRE(replyService.GetProperties() == properties);
    REQUIRE((replyService.GetAccount().m_id == firstProvider.m_id ||
      replyService.GetAccount().m_id == secondProvider.m_id));
    if(replyService.GetAccount().m_id == firstProvider.m_id) {
      REQUIRE(replyService.GetAccount().m_name == firstProvider.m_name);
    } else if(replyService.GetAccount().m_id == secondProvider.m_id) {
      REQUIRE(replyService.GetAccount().m_name == secondProvider.m_name);
    }
    replyService = services[1];
    REQUIRE(replyService.GetName() == service);
    REQUIRE(replyService.GetProperties() == properties);
    REQUIRE((replyService.GetAccount().m_id == firstProvider.m_id ||
      replyService.GetAccount().m_id == secondProvider.m_id));
    if(replyService.GetAccount().m_id == firstProvider.m_id) {
      REQUIRE(replyService.GetAccount().m_name == firstProvider.m_name);
    } else if(replyService.GetAccount().m_id == secondProvider.m_id) {
      REQUIRE(replyService.GetAccount().m_name == secondProvider.m_name);
    }
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      "new_account", "", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      "new_account", "", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  
    // Check that read only permission also results in an error.
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::READ);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      "new_account", "", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_unavailable_name") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto unavailableName = "unavailable";
    m_data_store.make_account(unavailableName, "",
      DirectoryEntry::get_star_directory(), second_clock::universal_time());
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      unavailableName, "", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_empty_name") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto accountName = "";
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      accountName, "", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_create_account") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto accountName = "account";
    m_client_protocol->send_request<MakeAccountService>(accountName, "",
      DirectoryEntry::get_star_directory());
    auto createdAccount = m_data_store.LoadAccount(accountName);
    REQUIRE((createdAccount.m_type == DirectoryEntry::Type::ACCOUNT));
    REQUIRE(createdAccount.m_name == accountName);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      "directory", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      "directory", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  
    // Check that read only permission also results in an error.
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::READ);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      "directory", DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_empty_name") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto directoryName = "";
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      directoryName, DirectoryEntry::get_star_directory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_create_directory") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto directoryName = "directory";
    m_client_protocol->send_request<MakeDirectoryService>(directoryName,
      DirectoryEntry::get_star_directory());
  }

  TEST_CASE_FIXTURE(Fixture, "delete_entry_without_login") {
    REQUIRE_THROWS_AS(
      m_client_protocol->send_request<DeleteDirectoryEntryService>(
      DirectoryEntry::get_star_directory()), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_non_existing_entry") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto entry = DirectoryEntry::MakeDirectory(1000, "");
    REQUIRE_THROWS_AS(
      m_client_protocol->send_request<DeleteDirectoryEntryService>(entry),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_account_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto deletedAccount = DirectoryEntry::make_account(0, "deleted_account");
    deletedAccount = m_data_store.make_account(deletedAccount.m_name, "",
      DirectoryEntry::get_star_directory(), second_clock::universal_time());
    REQUIRE_THROWS_AS(
      m_client_protocol->send_request<DeleteDirectoryEntryService>(
      deletedAccount), ServiceRequestException);
    REQUIRE(m_data_store.LoadAccount(deletedAccount.m_name) == deletedAccount);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_account") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto deletedAccount = DirectoryEntry::make_account(0, "deleted_account");
    deletedAccount = m_data_store.make_account(deletedAccount.m_name, "",
      DirectoryEntry::get_star_directory(), second_clock::universal_time());
    m_client_protocol->send_request<DeleteDirectoryEntryService>(deletedAccount);
    REQUIRE_THROWS_AS(m_data_store.LoadAccount(deletedAccount.m_name),
      ServiceLocatorDataStoreException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_directory_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto deletedDirectory = DirectoryEntry::MakeDirectory(0,
      "deleted_directory");
    deletedDirectory = m_data_store.MakeDirectory(deletedDirectory.m_name,
      DirectoryEntry::get_star_directory());
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      DeleteDirectoryEntryService>(deletedDirectory), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_directory") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::ADMINISTRATE);
    auto deletedDirectory = DirectoryEntry::MakeDirectory(0,
      "deleted_directory");
    deletedDirectory = m_data_store.MakeDirectory(deletedDirectory.m_name,
      DirectoryEntry::get_star_directory());
    m_client_protocol->send_request<DeleteDirectoryEntryService>(
      deletedDirectory);
  }

  TEST_CASE_FIXTURE(Fixture, "load_directory_entry_from_path") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      Permission::READ);
    auto a = m_data_store.MakeDirectory("a", DirectoryEntry::get_star_directory());
    auto b = m_data_store.MakeDirectory("b", a);
    auto c = m_data_store.MakeDirectory("c", b);
    auto result = m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::get_star_directory(), "a");
    REQUIRE(a == result);
    result = m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::get_star_directory(), "a/b");
    REQUIRE(b == result);
    result = m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::get_star_directory(), "a/b/c");
    REQUIRE(c == result);
    result = m_client_protocol->send_request<LoadPathService>(a, "b");
    REQUIRE(b == result);
    result = m_client_protocol->send_request<LoadPathService>(a, "b/c");
    REQUIRE(c == result);
    result = m_client_protocol->send_request<LoadPathService>(b, "c");
    REQUIRE(c == result);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoadPathService>(a, "c"),
      ServiceRequestException);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::get_star_directory(), "c"), ServiceRequestException);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::get_star_directory(), "phantom"), ServiceRequestException);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoadPathService>(c, ""),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_all_accounts") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(Store(account), Store(session_id));
    auto permissions = Permissions();
    permissions.Set(Permission::ADMINISTRATE);
    permissions.Set(Permission::READ);
    m_data_store.SetPermissions(account, DirectoryEntry::get_star_directory(),
      permissions);
    auto a = create_user("a", "");
    auto b = create_user("b", "");
    auto c = create_user("c", "");
    auto accounts = m_client_protocol->send_request<MonitorAccountsService>();
    auto expectedAccounts = std::vector<DirectoryEntry>();
    expectedAccounts.push_back(DirectoryEntry::GetRootAccount());
    expectedAccounts.push_back(account);
    expectedAccounts.push_back(a);
    expectedAccounts.push_back(b);
    expectedAccounts.push_back(c);
    REQUIRE(std::is_permutation(accounts.begin(), accounts.end(),
      expectedAccounts.begin(), expectedAccounts.end()));
    auto d = m_client_protocol->send_request<MakeAccountService>("d", "",
      DirectoryEntry::get_star_directory());
    auto updateMessage = std::dynamic_pointer_cast<RecordMessage<
      AccountUpdateMessage, Fixture::ClientServiceProtocolClient>>(
      m_client_protocol->ReadMessage());
    REQUIRE(updateMessage);
    REQUIRE(updateMessage->GetRecord().Get<0>() ==
      AccountUpdate{d, AccountUpdate::Type::ADDED});
    m_client_protocol->send_request<DeleteDirectoryEntryService>(b);
    updateMessage = std::dynamic_pointer_cast<RecordMessage<
      AccountUpdateMessage, Fixture::ClientServiceProtocolClient>>(
      m_client_protocol->ReadMessage());
    REQUIRE(updateMessage);
    REQUIRE(updateMessage->GetRecord().Get<0>() ==
      AccountUpdate{b, AccountUpdate::Type::DELETED});
  }
#endif
}
