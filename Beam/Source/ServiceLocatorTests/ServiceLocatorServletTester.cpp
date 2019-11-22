#include "Beam/ServiceLocatorTests/ServiceLocatorServletTester.hpp"
#include <boost/functional/factory.hpp>
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::property_tree;
using namespace std;

void ServiceLocatorServletTester::setUp() {
  m_dataStore = std::make_shared<LocalServiceLocatorDataStore>();
  m_serverConnection = std::make_shared<ServerConnection>();
  m_clientProtocol.emplace(Initialize("test", Ref(*m_serverConnection)),
    Initialize());
  RegisterServiceLocatorServices(Store(m_clientProtocol->GetSlots()));
  m_container.emplace(Initialize(m_dataStore), m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
}

void ServiceLocatorServletTester::tearDown() {
  m_clientProtocol = std::nullopt;
  m_container = std::nullopt;
  m_dataStore.reset();
}

void ServiceLocatorServletTester::TestLoginWithInvalidAccount() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LoginService>(
    "invalid_username", ""), ServiceRequestException);
}

void ServiceLocatorServletTester::TestLoginWithInvalidPassword() {
  string username = "user1";
  string password = "password";
  CreateUser(username, password);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LoginService>(username,
    "1234"), ServiceRequestException);
}

void ServiceLocatorServletTester::TestValidLogin() {
  string username = "user1";
  string password = "password";
  CreateUser(username, password);
  const LoginServiceResult& loginResult =
    m_clientProtocol->SendRequest<LoginService>(username, password);
  CPPUNIT_ASSERT(loginResult.account.m_name == username);
  CPPUNIT_ASSERT(loginResult.session_id.size() == SESSION_ID_LENGTH);

  // Test logging in again, duplicate login.
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LoginService>(username,
    password), ServiceRequestException);
}

void ServiceLocatorServletTester::TestAuthenticateAccountWithoutLogin() {
  string username = "user1";
  string password = "password";
  auto account = CreateUser(username, password);
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<
    AuthenticateAccountService>("invalid_user", "password"),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestAuthenticateAccountWithInvalidUsername() {
  string username = "user1";
  string password = "password";
  auto account = CreateUser(username, password);
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  auto loginResult = m_clientProtocol->SendRequest<LoginService>(
    username, password);
  auto result = m_clientProtocol->SendRequest<AuthenticateAccountService>(
    "invalid_user", "password");
  CPPUNIT_ASSERT(result.m_type == DirectoryEntry::Type::NONE);
  CPPUNIT_ASSERT(result.m_id == -1);
  CPPUNIT_ASSERT(result.m_name.empty());
}

void ServiceLocatorServletTester::TestAuthenticateAccountWithInvalidPassword() {
  string username = "user1";
  string password = "password";
  auto account = CreateUser(username, password);
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  auto loginResult = m_clientProtocol->SendRequest<LoginService>(
    username, password);
  auto result = m_clientProtocol->SendRequest<AuthenticateAccountService>(
    "user1", "invalid_password");
  CPPUNIT_ASSERT(result.m_type == DirectoryEntry::Type::NONE);
  CPPUNIT_ASSERT(result.m_id == -1);
  CPPUNIT_ASSERT(result.m_name.empty());
}

void ServiceLocatorServletTester::TestAuthenticateAccountWithoutPermission() {
  string username = "user1";
  string password = "password";
  auto account = CreateUser(username, password);
  auto loginResult = m_clientProtocol->SendRequest<LoginService>(
    username, password);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<
    AuthenticateAccountService>("user1", "password"), ServiceRequestException);
}

void ServiceLocatorServletTester::TestValidAuthenticateAccount() {
  string username = "user1";
  string password = "password";
  auto account = CreateUser(username, password);
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  auto loginResult = m_clientProtocol->SendRequest<LoginService>(
    username, password);
  auto result = m_clientProtocol->SendRequest<AuthenticateAccountService>(
    "user1", "password");
  CPPUNIT_ASSERT(result.m_type == DirectoryEntry::Type::ACCOUNT);
  CPPUNIT_ASSERT(result.m_id == loginResult.account.m_id);
  CPPUNIT_ASSERT(result.m_name == loginResult.account.m_name);
}

void ServiceLocatorServletTester::TestSessionAuthenticationWithoutLogin() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<
    SessionAuthenticationService>("", 0), ServiceRequestException);
}

void ServiceLocatorServletTester::
    TestSessionAuthenticationWithInvalidSessionId() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<
    SessionAuthenticationService>("", 0), ServiceRequestException);
}

void ServiceLocatorServletTester::TestValidSessionAuthentication() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  unsigned int key = 0;
  string encodedSessionId = ComputeSHA("0" + sessionId);
  DirectoryEntry result = m_clientProtocol->SendRequest<
    SessionAuthenticationService>(encodedSessionId, key);
  CPPUNIT_ASSERT(result == account);
}

void ServiceLocatorServletTester::TestRegisterServiceWithoutLogin() {
  string service = "service";
  JsonObject properties;
  properties["hostname"] = "tcp://localhost";
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<RegisterService>(service,
    properties), ServiceRequestException);
}

void ServiceLocatorServletTester::TestRegisterService() {
  string service = "service";
  JsonObject properties;
  properties["hostname"] = "tcp://localhost";
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  ServiceEntry result = m_clientProtocol->SendRequest<RegisterService>(service,
    properties);
  CPPUNIT_ASSERT(result.GetName() == service);
  CPPUNIT_ASSERT(result.GetProperties() == properties);
}

void ServiceLocatorServletTester::TestUnregisterServiceWithoutLogin() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<UnregisterService>(1),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestUnregisterInvalidService() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<UnregisterService>(1),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestRegisterUnregisterService() {
  string service = "service";
  JsonObject properties;
  properties["hostname"] = "tcp://localhost";

  // Login and register the service.
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  ServiceEntry registerResult = m_clientProtocol->SendRequest<RegisterService>(
    service, properties);
  CPPUNIT_ASSERT(registerResult.GetName() == service);
  CPPUNIT_ASSERT(registerResult.GetProperties() == properties);

  // Unregister the service.
  m_clientProtocol->SendRequest<UnregisterService>(registerResult.GetId());

  // Re-register the service.
  ServiceEntry reregisterResult =
    m_clientProtocol->SendRequest<RegisterService>(service, properties);
  CPPUNIT_ASSERT(reregisterResult.GetName() == service);
  CPPUNIT_ASSERT(reregisterResult.GetProperties() == properties);
  CPPUNIT_ASSERT(reregisterResult.GetId() != registerResult.GetId());
}

void ServiceLocatorServletTester::TestLocatingWithoutLogin() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LocateService>(""),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestLocatingNonExistingService() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  vector<ServiceEntry> result =
    m_clientProtocol->SendRequest<LocateService>("");
  CPPUNIT_ASSERT(result.empty());
}

void ServiceLocatorServletTester::TestLocatingSingleProviderService() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  std::optional<ClientServiceProtocolClient> providerService;
  DirectoryEntry provider;
  string providerSession;
  CreateAdditionalClient("user2", "password", Store(provider),
    Store(providerSession), Store(providerService));

  // Register the service using the mock provider.
  JsonObject properties;
  properties["hostname"] = "tcp://localhost";
  ServiceEntry registerResult = providerService->SendRequest<RegisterService>(
    "service", properties);

  // Locate the service using the mock client.
  vector<ServiceEntry> locateResult =
    m_clientProtocol->SendRequest<LocateService>("service");
  CPPUNIT_ASSERT(locateResult.size() == 1);
  const ServiceEntry& replyService = locateResult.front();
  CPPUNIT_ASSERT(replyService.GetName() == "service");
  CPPUNIT_ASSERT(replyService.GetAccount().m_name == provider.m_name);
  CPPUNIT_ASSERT(replyService.GetAccount().m_id == provider.m_id);

  // Unregister the service.
  providerService->SendRequest<UnregisterService>(replyService.GetId());

  // Redo the locate.
  vector<ServiceEntry> relocateResult =
    m_clientProtocol->SendRequest<LocateService>("service");
  CPPUNIT_ASSERT(relocateResult.empty());
}

void ServiceLocatorServletTester::TestLocatingMultipleProviderService() {
  string service = "service";
  JsonObject properties;
  properties["hostname"] = "tcp://localhost";
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  std::optional<ClientServiceProtocolClient> providerService;
  DirectoryEntry firstProvider;
  string firstProviderSession;
  CreateAdditionalClient("user2", "password", Store(firstProvider),
    Store(firstProviderSession), Store(providerService));

  // Register the service using the first provider.
  int firstServiceId = 1;
  int secondServiceId = 2;
  ServiceEntry registerResult = providerService->SendRequest<RegisterService>(
    service, properties);
  CPPUNIT_ASSERT(registerResult.GetName() == service);
  CPPUNIT_ASSERT(registerResult.GetProperties() == properties);

  // Register the service using the second provider.
  std::optional<ClientServiceProtocolClient> secondaryProviderService;
  DirectoryEntry secondProvider;
  string secondProviderSession;
  CreateAdditionalClient("user3", "password", Store(secondProvider),
    Store(secondProviderSession), Store(secondaryProviderService));
  ServiceEntry secondRegisterResult =
    secondaryProviderService->SendRequest<RegisterService>(service, properties);
  CPPUNIT_ASSERT(secondRegisterResult.GetName() == service);
  CPPUNIT_ASSERT(secondRegisterResult.GetProperties() == properties);

  // Locate the service using the mock client.
  vector<ServiceEntry> services = m_clientProtocol->SendRequest<LocateService>(
    "service");
  CPPUNIT_ASSERT(services.size() == 2);
  CPPUNIT_ASSERT(services[0].GetAccount().m_id !=
    services[1].GetAccount().m_id);
  ServiceEntry replyService = services[0];
  CPPUNIT_ASSERT(replyService.GetName() == service);
  CPPUNIT_ASSERT(replyService.GetProperties() == properties);
  CPPUNIT_ASSERT(replyService.GetAccount().m_id == firstProvider.m_id ||
    replyService.GetAccount().m_id == secondProvider.m_id);
  if(replyService.GetAccount().m_id == firstProvider.m_id) {
    CPPUNIT_ASSERT(replyService.GetAccount().m_name == firstProvider.m_name);
  } else if(replyService.GetAccount().m_id == secondProvider.m_id) {
    CPPUNIT_ASSERT(replyService.GetAccount().m_name == secondProvider.m_name);
  }
  replyService = services[1];
  CPPUNIT_ASSERT(replyService.GetName() == service);
  CPPUNIT_ASSERT(replyService.GetProperties() == properties);
  CPPUNIT_ASSERT(replyService.GetAccount().m_id == firstProvider.m_id ||
    replyService.GetAccount().m_id == secondProvider.m_id);
  if(replyService.GetAccount().m_id == firstProvider.m_id) {
    CPPUNIT_ASSERT(replyService.GetAccount().m_name == firstProvider.m_name);
  } else if(replyService.GetAccount().m_id == secondProvider.m_id) {
    CPPUNIT_ASSERT(replyService.GetAccount().m_name == secondProvider.m_name);
  }
}

void ServiceLocatorServletTester::TestCreateAccountWithoutLogin() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeAccountService>(
    "new_account", "", DirectoryEntry::GetStarDirectory()),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestCreateAccountWithoutPermissions() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeAccountService>(
    "new_account", "", DirectoryEntry::GetStarDirectory()),
    ServiceRequestException);

  // Check that read only permission also results in an error.
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::READ);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeAccountService>(
    "new_account", "", DirectoryEntry::GetStarDirectory()),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestCreateAccountUnavailableName() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  string unavailableName = "unavailable";
  m_dataStore->MakeAccount(unavailableName, "",
    DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeAccountService>(
    unavailableName, "", DirectoryEntry::GetStarDirectory()),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestCreateAccountEmptyName() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  string accountName = "";
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeAccountService>(
    accountName, "", DirectoryEntry::GetStarDirectory()),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestValidCreateAccount() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  string accountName = "account";
  m_clientProtocol->SendRequest<MakeAccountService>(accountName, "",
    DirectoryEntry::GetStarDirectory());
  DirectoryEntry createdAccount = m_dataStore->LoadAccount(accountName);
  CPPUNIT_ASSERT(createdAccount.m_type == DirectoryEntry::Type::ACCOUNT);
  CPPUNIT_ASSERT(createdAccount.m_name == accountName);
}

void ServiceLocatorServletTester::TestCreateDirectoryWithoutLogin() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeDirectoryService>(
    "directory", DirectoryEntry::GetStarDirectory()), ServiceRequestException);
}

void ServiceLocatorServletTester::TestCreateDirectoryWithoutPermissions() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeDirectoryService>(
    "directory", DirectoryEntry::GetStarDirectory()), ServiceRequestException);

  // Check that read only permission also results in an error.
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::READ);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeDirectoryService>(
    "directory", DirectoryEntry::GetStarDirectory()), ServiceRequestException);
}

void ServiceLocatorServletTester::TestCreateDirectoryEmptyName() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  string directoryName = "";
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<MakeDirectoryService>(
    directoryName, DirectoryEntry::GetStarDirectory()),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestValidCreateDirectory() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  string directoryName = "directory";
  m_clientProtocol->SendRequest<MakeDirectoryService>(directoryName,
    DirectoryEntry::GetStarDirectory());
}

void ServiceLocatorServletTester::TestDeleteEntryWithoutLogin() {
  CPPUNIT_ASSERT_THROW(
    m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(
    DirectoryEntry::GetStarDirectory()), ServiceRequestException);
}

void ServiceLocatorServletTester::TestDeleteNonExistingEntry() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  DirectoryEntry entry(DirectoryEntry::Type::DIRECTORY, 1000, "");
  CPPUNIT_ASSERT_THROW(
    m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(entry),
    ServiceRequestException);
}

void ServiceLocatorServletTester::TestDeleteAccountWithoutPermissions() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  DirectoryEntry deletedAccount(DirectoryEntry::Type::ACCOUNT, 0,
    "deleted_account");
  deletedAccount = m_dataStore->MakeAccount(deletedAccount.m_name, "",
    DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
  CPPUNIT_ASSERT_THROW(
    m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(deletedAccount),
    ServiceRequestException);
  CPPUNIT_ASSERT(m_dataStore->LoadAccount(deletedAccount.m_name) ==
    deletedAccount);
}

void ServiceLocatorServletTester::TestDeleteAccount() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  DirectoryEntry deletedAccount(DirectoryEntry::Type::ACCOUNT, 0,
    "deleted_account");
  deletedAccount = m_dataStore->MakeAccount(deletedAccount.m_name, "",
    DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
  m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(deletedAccount);
  CPPUNIT_ASSERT_THROW(m_dataStore->LoadAccount(deletedAccount.m_name),
    ServiceLocatorDataStoreException);
}

void ServiceLocatorServletTester::TestDeleteDirectoryWithoutPermissions() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  DirectoryEntry deletedDirectory(DirectoryEntry::Type::DIRECTORY, 0,
    "deleted_directory");
  deletedDirectory = m_dataStore->MakeDirectory(deletedDirectory.m_name,
    DirectoryEntry::GetStarDirectory());
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<
    DeleteDirectoryEntryService>(deletedDirectory), ServiceRequestException);
}

void ServiceLocatorServletTester::TestDeleteDirectory() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::ADMINISTRATE);
  DirectoryEntry deletedDirectory(DirectoryEntry::Type::DIRECTORY, 0,
    "deleted_directory");
  deletedDirectory = m_dataStore->MakeDirectory(deletedDirectory.m_name,
    DirectoryEntry::GetStarDirectory());
  m_clientProtocol->SendRequest<DeleteDirectoryEntryService>(deletedDirectory);
}

void ServiceLocatorServletTester::TestLoadDirectoryEntryFromPath() {
  DirectoryEntry account;
  string sessionId;
  CreateAccountAndLogin(Store(account), Store(sessionId));
  m_dataStore->SetPermissions(account, DirectoryEntry::GetStarDirectory(),
    Permission::READ);
  DirectoryEntry a = m_dataStore->MakeDirectory("a",
    DirectoryEntry::GetStarDirectory());
  DirectoryEntry b = m_dataStore->MakeDirectory("b", a);
  DirectoryEntry c = m_dataStore->MakeDirectory("c", b);
  DirectoryEntry result;
  result = m_clientProtocol->SendRequest<LoadPathService>(
    DirectoryEntry::GetStarDirectory(), "a");
  CPPUNIT_ASSERT(a == result);
  result = m_clientProtocol->SendRequest<LoadPathService>(
    DirectoryEntry::GetStarDirectory(), "a/b");
  CPPUNIT_ASSERT(b == result);
  result = m_clientProtocol->SendRequest<LoadPathService>(
    DirectoryEntry::GetStarDirectory(), "a/b/c");
  CPPUNIT_ASSERT(c == result);
  result = m_clientProtocol->SendRequest<LoadPathService>(a, "b");
  CPPUNIT_ASSERT(b == result);
  result = m_clientProtocol->SendRequest<LoadPathService>(a, "b/c");
  CPPUNIT_ASSERT(c == result);
  result = m_clientProtocol->SendRequest<LoadPathService>(b, "c");
  CPPUNIT_ASSERT(c == result);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LoadPathService>(a, "c"),
    ServiceRequestException);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LoadPathService>(
    DirectoryEntry::GetStarDirectory(), "c"), ServiceRequestException);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LoadPathService>(
    DirectoryEntry::GetStarDirectory(), "phantom"), ServiceRequestException);
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<LoadPathService>(c, ""),
    ServiceRequestException);
}

DirectoryEntry ServiceLocatorServletTester::CreateUser(const string& username,
    const string& password) {
  DirectoryEntry account = m_dataStore->MakeAccount(username, password,
    DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
  CPPUNIT_ASSERT(account.m_id != -1);
  return account;
}

void ServiceLocatorServletTester::CreateAccountAndLogin(
    Out<DirectoryEntry> account, Out<string> sessionId) {
  string username = "user1";
  string password = "password";
  CreateUser(username, password);
  LoginServiceResult result = m_clientProtocol->SendRequest<LoginService>(
    username, password);
  *account = result.account;
  *sessionId = result.session_id;
}

void ServiceLocatorServletTester::CreateAdditionalClient(const string& username,
    const string& password, Out<DirectoryEntry> account, Out<string> sessionId,
    Out<std::optional<ClientServiceProtocolClient>> service) {
  service->emplace(Initialize(string("test"), Ref(*m_serverConnection)),
    Initialize());
  RegisterServiceLocatorServices(Store((*service)->GetSlots()));
  (*service)->Open();
  CreateUser(username, password);
  LoginServiceResult result = (*service)->SendRequest<LoginService>(username,
    password);
  *account = result.account;
  *sessionId = result.session_id;
}
