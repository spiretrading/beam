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
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::property_tree;

namespace {
  struct Fixture {
    using ServerConnection = LocalServerConnection<SharedBuffer>;
    using TestServiceProtocolServletContainer = ServiceProtocolServletContainer<
      MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>,
      ServerConnection*, BinarySender<SharedBuffer>, NullEncoder,
      std::shared_ptr<TriggerTimer>>;
    using ClientChannel = LocalClientChannel<SharedBuffer>;
    using ClientServiceProtocolClient = ServiceProtocolClient<
      MessageProtocol<ClientChannel, BinarySender<SharedBuffer>, NullEncoder>,
      TriggerTimer>;

    LocalServiceLocatorDataStore m_dataStore;
    ServerConnection m_serverConnection;
    TestServiceProtocolServletContainer m_container;
    optional<ClientServiceProtocolClient> m_clientProtocol;

    Fixture()
        : m_container(Initialize(&m_dataStore), &m_serverConnection,
            factory<std::shared_ptr<TriggerTimer>>()) {
      m_clientProtocol.emplace(Initialize("test", m_serverConnection),
        Initialize());
      RegisterServiceLocatorServices(Store(m_clientProtocol->GetSlots()));
      RegisterServiceLocatorMessages(Store(m_clientProtocol->GetSlots()));
    }

    DirectoryEntry CreateUser(const std::string& username,
        const std::string& password) {
      auto account = m_dataStore.MakeAccount(username, password,
        DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
      REQUIRE(account.m_id != -1);
      return account;
    }

    void CreateAccountAndLogin(Out<DirectoryEntry> account,
        Out<std::string> sessionId) {
      auto username = std::string("user1");
      auto password = std::string("password");
      CreateUser(username, password);
      auto result = m_clientProtocol->SendRequest<LoginService>(username,
        password);
      *account = result.account;
      *sessionId = result.session_id;
    }

    void CreateAdditionalClient(const std::string& username,
        const std::string& password, Out<DirectoryEntry> account,
        Out<std::string> sessionId,
        Out<std::optional<ClientServiceProtocolClient>> service) {
      service->emplace(Initialize("test", m_serverConnection), Initialize());
      RegisterServiceLocatorServices(Store((*service)->GetSlots()));
      CreateUser(username, password);
      auto result = (*service)->SendRequest<LoginService>(username, password);
      *account = result.account;
      *sessionId = result.session_id;
    }
  };
}

TEST_SUITE("ServiceLocatorServlet") {
  TEST_CASE_FIXTURE(Fixture, "login_with_invalid_account") {
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LoginService>(
      "invalid_username", ""), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "login_with_invalid_password") {
    auto username = std::string("user1");
    auto password = std::string("password");
    CreateUser(username, password);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LoginService>(username,
      "1234"), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_login") {
    auto username = std::string("user1");
    auto password = std::string("password");
    CreateUser(username, password);
    auto loginResult = m_clientProtocol->SendRequest<LoginService>(username,
      password);
    REQUIRE(loginResult.account.m_name == username);
    REQUIRE(loginResult.session_id.size() == SESSION_ID_LENGTH);

    // Test logging in again, duplicate login.
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LoginService>(username,
      password), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_without_login") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = CreateUser(username, password);
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<
      AuthenticateAccountService>("invalid_user", "password"),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_with_invalid_username") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = CreateUser(username, password);
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto loginResult = m_clientProtocol->SendRequest<LoginService>(
      username, password);
    auto result = m_clientProtocol->SendRequest<AuthenticateAccountService>(
      "invalid_user", "password");
    REQUIRE(result.m_type == DirectoryEntry::Type::NONE);
    REQUIRE(result.m_id == -1);
    REQUIRE(result.m_name.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_with_invalid_password") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = CreateUser(username, password);
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto loginResult = m_clientProtocol->SendRequest<LoginService>(
      username, password);
    auto result = m_clientProtocol->SendRequest<AuthenticateAccountService>(
      "user1", "invalid_password");
    REQUIRE(result.m_type == DirectoryEntry::Type::NONE);
    REQUIRE(result.m_id == -1);
    REQUIRE(result.m_name.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_without_permission") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = CreateUser(username, password);
    auto loginResult = m_clientProtocol->SendRequest<LoginService>(
      username, password);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<
      AuthenticateAccountService>("user1", "password"),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_authenticate_account") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = CreateUser(username, password);
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto loginResult = m_clientProtocol->SendRequest<LoginService>(
      username, password);
    auto result = m_clientProtocol->SendRequest<AuthenticateAccountService>(
      "user1", "password");
    REQUIRE(result.m_type == DirectoryEntry::Type::ACCOUNT);
    REQUIRE(result.m_id == loginResult.account.m_id);
    REQUIRE(result.m_name == loginResult.account.m_name);
  }

  TEST_CASE_FIXTURE(Fixture, "session_authentication_without_login") {
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<
      SessionAuthenticationService>("", 0), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "session_authentication_with_invalid_session_id") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<
      SessionAuthenticationService>("", 0), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_session_authentication") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto key = 0U;
    auto encodedSessionId = ComputeSHA("0" + sessionId);
    auto result = m_clientProtocol->SendRequest<SessionAuthenticationService>(
      encodedSessionId, key);
    REQUIRE(result == account);
  }

  TEST_CASE_FIXTURE(Fixture, "register_service_without_login") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<RegisterService>(service,
      properties), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "register_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto result = m_clientProtocol->SendRequest<RegisterService>(service,
      properties);
    REQUIRE(result.GetName() == service);
    REQUIRE(result.GetProperties() == properties);
  }

  TEST_CASE_FIXTURE(Fixture, "unregister_service_without_login") {
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<UnregisterService>(1),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "unregister_invalid_service") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<UnregisterService>(1),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "register_unregister_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
  
    // Login and register the service.
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto registerResult = m_clientProtocol->SendRequest<RegisterService>(
      service, properties);
    REQUIRE(registerResult.GetName() == service);
    REQUIRE(registerResult.GetProperties() == properties);
  
    // Unregister the service.
    m_clientProtocol->SendRequest<UnregisterService>(registerResult.GetId());
  
    // Re-register the service.
    auto reregisterResult = m_clientProtocol->SendRequest<RegisterService>(
      service, properties);
    REQUIRE(reregisterResult.GetName() == service);
    REQUIRE(reregisterResult.GetProperties() == properties);
    REQUIRE(reregisterResult.GetId() != registerResult.GetId());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_without_login") {
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LocateService>(""),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "locating_non_existing_service") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto result = m_clientProtocol->SendRequest<LocateService>("");
    REQUIRE(result.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_single_provider_service") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto providerService = std::optional<ClientServiceProtocolClient>();
    auto provider = DirectoryEntry();
    auto providerSession = std::string();
    CreateAdditionalClient("user2", "password", Store(provider),
      Store(providerSession), Store(providerService));
  
    // Register the service using the mock provider.
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto registerResult = providerService->SendRequest<RegisterService>(
      "service", properties);
  
    // Locate the service using the mock client.
    auto locateResult = m_clientProtocol->SendRequest<LocateService>("service");
    REQUIRE(locateResult.size() == 1);
    auto& replyService = locateResult.front();
    REQUIRE(replyService.GetName() == "service");
    REQUIRE(replyService.GetAccount().m_name == provider.m_name);
    REQUIRE(replyService.GetAccount().m_id == provider.m_id);
  
    // Unregister the service.
    providerService->SendRequest<UnregisterService>(replyService.GetId());
  
    // Redo the locate.
    auto relocateResult = m_clientProtocol->SendRequest<LocateService>(
      "service");
    REQUIRE(relocateResult.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_multiple_provider_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto providerService = std::optional<ClientServiceProtocolClient>();
    auto firstProvider = DirectoryEntry();
    auto firstProviderSession = std::string();
    CreateAdditionalClient("user2", "password", Store(firstProvider),
      Store(firstProviderSession), Store(providerService));
  
    // Register the service using the first provider.
    auto firstServiceId = 1;
    auto secondServiceId = 2;
    auto registerResult = providerService->SendRequest<RegisterService>(service,
      properties);
    REQUIRE(registerResult.GetName() == service);
    REQUIRE(registerResult.GetProperties() == properties);
  
    // Register the service using the second provider.
    auto secondaryProviderService =
      std::optional<ClientServiceProtocolClient>();
    auto secondProvider = DirectoryEntry();
    auto secondProviderSession = std::string();
    CreateAdditionalClient("user3", "password", Store(secondProvider),
      Store(secondProviderSession), Store(secondaryProviderService));
    auto secondRegisterResult =
      secondaryProviderService->SendRequest<RegisterService>(service,
      properties);
    REQUIRE(secondRegisterResult.GetName() == service);
    REQUIRE(secondRegisterResult.GetProperties() == properties);
  
    // Locate the service using the mock client.
    auto services = m_clientProtocol->SendRequest<LocateService>("service");
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
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeAccountService>(
      "new_account", "", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_without_permissions") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeAccountService>(
      "new_account", "", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  
    // Check that read only permission also results in an error.
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::READ);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeAccountService>(
      "new_account", "", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_unavailable_name") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto unavailableName = "unavailable";
    m_dataStore.MakeAccount(unavailableName, "",
      DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeAccountService>(
      unavailableName, "", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_empty_name") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto accountName = "";
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeAccountService>(
      accountName, "", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_create_account") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto accountName = "account";
    m_clientProtocol->SendRequest<MakeAccountService>(accountName, "",
      DirectoryEntry::GetStarDirectory());
    auto createdAccount = m_dataStore.LoadAccount(accountName);
    REQUIRE(createdAccount.m_type == DirectoryEntry::Type::ACCOUNT);
    REQUIRE(createdAccount.m_name == accountName);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_without_login") {
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeDirectoryService>(
      "directory", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_without_permissions") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeDirectoryService>(
      "directory", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  
    // Check that read only permission also results in an error.
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::READ);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeDirectoryService>(
      "directory", DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_empty_name") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto directoryName = "";
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<MakeDirectoryService>(
      directoryName, DirectoryEntry::GetStarDirectory()),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_create_directory") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto directoryName = "directory";
    m_clientProtocol->SendRequest<MakeDirectoryService>(directoryName,
      DirectoryEntry::GetStarDirectory());
  }

  TEST_CASE_FIXTURE(Fixture, "delete_entry_without_login") {
    REQUIRE_THROWS_AS(
      m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(
      DirectoryEntry::GetStarDirectory()), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_non_existing_entry") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto entry = DirectoryEntry::MakeDirectory(1000, "");
    REQUIRE_THROWS_AS(
      m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(entry),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_account_without_permissions") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto deletedAccount = DirectoryEntry::MakeAccount(0, "deleted_account");
    deletedAccount = m_dataStore.MakeAccount(deletedAccount.m_name, "",
      DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
    REQUIRE_THROWS_AS(
      m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(
      deletedAccount), ServiceRequestException);
    REQUIRE(m_dataStore.LoadAccount(deletedAccount.m_name) == deletedAccount);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_account") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto deletedAccount = DirectoryEntry::MakeAccount(0, "deleted_account");
    deletedAccount = m_dataStore.MakeAccount(deletedAccount.m_name, "",
      DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
    m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(deletedAccount);
    REQUIRE_THROWS_AS(m_dataStore.LoadAccount(deletedAccount.m_name),
      ServiceLocatorDataStoreException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_directory_without_permissions") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto deletedDirectory = DirectoryEntry::MakeDirectory(0,
      "deleted_directory");
    deletedDirectory = m_dataStore.MakeDirectory(deletedDirectory.m_name,
      DirectoryEntry::GetStarDirectory());
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<
      DeleteDirectoryEntryService>(deletedDirectory), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_directory") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::ADMINISTRATE);
    auto deletedDirectory = DirectoryEntry::MakeDirectory(0,
      "deleted_directory");
    deletedDirectory = m_dataStore.MakeDirectory(deletedDirectory.m_name,
      DirectoryEntry::GetStarDirectory());
    m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(
      deletedDirectory);
  }

  TEST_CASE_FIXTURE(Fixture, "load_directory_entry_from_path") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      Permission::READ);
    auto a = m_dataStore.MakeDirectory("a", DirectoryEntry::GetStarDirectory());
    auto b = m_dataStore.MakeDirectory("b", a);
    auto c = m_dataStore.MakeDirectory("c", b);
    auto result = m_clientProtocol->SendRequest<LoadPathService>(
      DirectoryEntry::GetStarDirectory(), "a");
    REQUIRE(a == result);
    result = m_clientProtocol->SendRequest<LoadPathService>(
      DirectoryEntry::GetStarDirectory(), "a/b");
    REQUIRE(b == result);
    result = m_clientProtocol->SendRequest<LoadPathService>(
      DirectoryEntry::GetStarDirectory(), "a/b/c");
    REQUIRE(c == result);
    result = m_clientProtocol->SendRequest<LoadPathService>(a, "b");
    REQUIRE(b == result);
    result = m_clientProtocol->SendRequest<LoadPathService>(a, "b/c");
    REQUIRE(c == result);
    result = m_clientProtocol->SendRequest<LoadPathService>(b, "c");
    REQUIRE(c == result);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LoadPathService>(a, "c"),
      ServiceRequestException);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LoadPathService>(
      DirectoryEntry::GetStarDirectory(), "c"), ServiceRequestException);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LoadPathService>(
      DirectoryEntry::GetStarDirectory(), "phantom"), ServiceRequestException);
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<LoadPathService>(c, ""),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_all_accounts") {
    auto account = DirectoryEntry();
    auto sessionId = std::string();
    CreateAccountAndLogin(Store(account), Store(sessionId));
    auto permissions = Permissions();
    permissions.Set(Permission::ADMINISTRATE);
    permissions.Set(Permission::READ);
    m_dataStore.SetPermissions(account, DirectoryEntry::GetStarDirectory(),
      permissions);
    auto a = CreateUser("a", "");
    auto b = CreateUser("b", "");
    auto c = CreateUser("c", "");
    auto accounts = m_clientProtocol->SendRequest<MonitorAccountsService>();
    auto expectedAccounts = std::vector<DirectoryEntry>();
    expectedAccounts.push_back(DirectoryEntry::GetRootAccount());
    expectedAccounts.push_back(account);
    expectedAccounts.push_back(a);
    expectedAccounts.push_back(b);
    expectedAccounts.push_back(c);
    REQUIRE(std::is_permutation(accounts.begin(), accounts.end(),
      expectedAccounts.begin(), expectedAccounts.end()));
    auto d = m_clientProtocol->SendRequest<MakeAccountService>("d", "",
      DirectoryEntry::GetStarDirectory());
    auto updateMessage = std::dynamic_pointer_cast<RecordMessage<
      AccountUpdateMessage, Fixture::ClientServiceProtocolClient>>(
      m_clientProtocol->ReadMessage());
    REQUIRE(updateMessage != nullptr);
    REQUIRE(updateMessage->GetRecord().Get<0>() ==
      AccountUpdate{d, AccountUpdate::Type::ADDED});
    m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(b);
    updateMessage = std::dynamic_pointer_cast<RecordMessage<
      AccountUpdateMessage, Fixture::ClientServiceProtocolClient>>(
      m_clientProtocol->ReadMessage());
    REQUIRE(updateMessage != nullptr);
    REQUIRE(updateMessage->GetRecord().Get<0>() ==
      AccountUpdate{b, AccountUpdate::Type::DELETED});
  }
}
