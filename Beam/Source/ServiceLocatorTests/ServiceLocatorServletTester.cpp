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
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoginService>(
      username, "1234"), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_login") {
    auto username = std::string("user1");
    auto password = std::string("password");
    create_user(username, password);
    auto loginResult =
      m_client_protocol->send_request<LoginService>(username, password);
    REQUIRE(loginResult.account.m_name == username);
    REQUIRE(loginResult.session_id.size() == SESSION_ID_LENGTH);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoginService>(
      username, password), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "login_from_session_without_login") {
    REQUIRE_THROWS_AS(
      m_client_protocol->send_request<LoginFromSessionService>("", 0),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "login_from_session_with_invalid_session") {
    auto source_client = std::optional<ClientServiceProtocolClient>();
    source_client.emplace(init("test", m_server_connection), init());
    register_service_locator_services(out(source_client->get_slots()));
    create_user("user1", "password");
    auto login_result =
      source_client->send_request<LoginService>("user1", "password");
    auto key = generate_encryption_key();
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoginFromSessionService>(
      "invalid_session", key), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_login_from_session") {
    auto source_client = std::optional<ClientServiceProtocolClient>();
    source_client.emplace(init("test", m_server_connection), init());
    register_service_locator_services(out(source_client->get_slots()));
    create_user("user1", "password");
    auto login_result =
      source_client->send_request<LoginService>("user1", "password");
    auto key = generate_encryption_key();
    auto encrypted_session_id =
      compute_sha(std::to_string(key) + login_result.session_id);
    auto result = m_client_protocol->send_request<LoginFromSessionService>(
      encrypted_session_id, key);
    REQUIRE(result.account == login_result.account);
    REQUIRE(result.session_id.size() == SESSION_ID_LENGTH);
    REQUIRE(result.session_id != login_result.session_id);
  }

  TEST_CASE_FIXTURE(Fixture, "login_from_session_duplicate_login") {
    auto source_client = std::optional<ClientServiceProtocolClient>();
    source_client.emplace(init("test", m_server_connection), init());
    register_service_locator_services(out(source_client->get_slots()));
    create_user("user1", "password");
    auto login_result =
      source_client->send_request<LoginService>("user1", "password");
    auto key = generate_encryption_key();
    auto encrypted_session_id =
      compute_sha(std::to_string(key) + login_result.session_id);
    m_client_protocol->send_request<LoginFromSessionService>(
      encrypted_session_id, key);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoginFromSessionService>(
      encrypted_session_id, key), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "login_from_session_is_independent") {
    auto source_client = std::optional<ClientServiceProtocolClient>();
    source_client.emplace(init("test", m_server_connection), init());
    register_service_locator_services(out(source_client->get_slots()));
    create_user("user1", "password");
    auto login_result =
      source_client->send_request<LoginService>("user1", "password");
    auto key = generate_encryption_key();
    auto encrypted_session_id =
      compute_sha(std::to_string(key) + login_result.session_id);
    auto result = m_client_protocol->send_request<LoginFromSessionService>(
      encrypted_session_id, key);
    m_data_store.set_permissions(
      result.account, DirectoryEntry::STAR_DIRECTORY, Permission::READ);
    auto children = m_client_protocol->send_request<LoadChildrenService>(
      DirectoryEntry::STAR_DIRECTORY);
    REQUIRE_FALSE(children.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_without_login") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      AuthenticateAccountService>("invalid_user", "password"),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "authenticate_account_with_invalid_username") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto login_result =
      m_client_protocol->send_request<LoginService>(username, password);
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
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto login_result =
      m_client_protocol->send_request<LoginService>(username, password);
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
    auto login_result =
      m_client_protocol->send_request<LoginService>(username, password);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      AuthenticateAccountService>("user1", "password"),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_authenticate_account") {
    auto username = std::string("user1");
    auto password = std::string("password");
    auto account = create_user(username, password);
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto login_result =
      m_client_protocol->send_request<LoginService>(username, password);
    auto result = m_client_protocol->send_request<AuthenticateAccountService>(
      "user1", "password");
    REQUIRE((result.m_type == DirectoryEntry::Type::ACCOUNT));
    REQUIRE(result.m_id == login_result.account.m_id);
    REQUIRE(result.m_name == login_result.account.m_name);
  }

  TEST_CASE_FIXTURE(Fixture, "session_authentication_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      SessionAuthenticationService>("", 0), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "session_authentication_with_invalid_session_id") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      SessionAuthenticationService>("", 0), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_session_authentication") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto key = 0U;
    auto encrypted_session_id = compute_sha("0" + session_id);
    auto result = m_client_protocol->send_request<SessionAuthenticationService>(
      encrypted_session_id, key);
    REQUIRE(result == account);
  }

  TEST_CASE_FIXTURE(Fixture, "register_service_without_login") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    REQUIRE_THROWS_AS(m_client_protocol->send_request<RegisterService>(
      service, properties), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "register_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto result =
      m_client_protocol->send_request<RegisterService>(service, properties);
    REQUIRE(result.get_name() == service);
    REQUIRE(result.get_properties() == properties);
  }

  TEST_CASE_FIXTURE(Fixture, "unregister_service_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<UnregisterService>(1),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "unregister_invalid_service") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<UnregisterService>(1),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "register_unregister_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto register_result =
      m_client_protocol->send_request<RegisterService>(service, properties);
    REQUIRE(register_result.get_name() == service);
    REQUIRE(register_result.get_properties() == properties);
    m_client_protocol->send_request<UnregisterService>(
      register_result.get_id());
    auto reregister_result =
      m_client_protocol->send_request<RegisterService>(service, properties);
    REQUIRE(reregister_result.get_name() == service);
    REQUIRE(reregister_result.get_properties() == properties);
    REQUIRE(reregister_result.get_id() != register_result.get_id());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LocateService>(""),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "locating_non_existing_service") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto result = m_client_protocol->send_request<LocateService>("");
    REQUIRE(result.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_single_provider_service") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto provider_service = std::optional<ClientServiceProtocolClient>();
    auto provider = DirectoryEntry();
    auto provider_session = std::string();
    create_additional_client("user2", "password", out(provider),
      out(provider_session), out(provider_service));
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto register_result =
      provider_service->send_request<RegisterService>("service", properties);
    auto locate_result =
      m_client_protocol->send_request<LocateService>("service");
    REQUIRE(locate_result.size() == 1);
    auto& reply_service = locate_result.front();
    REQUIRE(reply_service.get_name() == "service");
    REQUIRE(reply_service.get_account().m_name == provider.m_name);
    REQUIRE(reply_service.get_account().m_id == provider.m_id);
    provider_service->send_request<UnregisterService>(reply_service.get_id());
    auto relocate_result =
      m_client_protocol->send_request<LocateService>("service");
    REQUIRE(relocate_result.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "locating_multiple_provider_service") {
    auto service = "service";
    auto properties = JsonObject();
    properties["hostname"] = "tcp://localhost";
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto provider_service = std::optional<ClientServiceProtocolClient>();
    auto first_provider = DirectoryEntry();
    auto first_provider_session = std::string();
    create_additional_client("user2", "password", out(first_provider),
      out(first_provider_session), out(provider_service));
    auto register_result =
      provider_service->send_request<RegisterService>(service, properties);
    REQUIRE(register_result.get_name() == service);
    REQUIRE(register_result.get_properties() == properties);
    auto secondary_provider_service =
      std::optional<ClientServiceProtocolClient>();
    auto second_provider = DirectoryEntry();
    auto second_provider_session = std::string();
    create_additional_client("user3", "password", out(second_provider),
      out(second_provider_session), out(secondary_provider_service));
    auto second_register_result =
      secondary_provider_service->send_request<RegisterService>(
        service, properties);
    REQUIRE(second_register_result.get_name() == service);
    REQUIRE(second_register_result.get_properties() == properties);
    auto services = m_client_protocol->send_request<LocateService>("service");
    REQUIRE(services.size() == 2);
    REQUIRE(services[0].get_account().m_id != services[1].get_account().m_id);
    auto reply_service = services[0];
    REQUIRE(reply_service.get_name() == service);
    REQUIRE(reply_service.get_properties() == properties);
    REQUIRE((reply_service.get_account().m_id == first_provider.m_id ||
      reply_service.get_account().m_id == second_provider.m_id));
    if(reply_service.get_account().m_id == first_provider.m_id) {
      REQUIRE(reply_service.get_account().m_name == first_provider.m_name);
    } else if(reply_service.get_account().m_id == second_provider.m_id) {
      REQUIRE(reply_service.get_account().m_name == second_provider.m_name);
    }
    reply_service = services[1];
    REQUIRE(reply_service.get_name() == service);
    REQUIRE(reply_service.get_properties() == properties);
    REQUIRE((reply_service.get_account().m_id == first_provider.m_id ||
      reply_service.get_account().m_id == second_provider.m_id));
    if(reply_service.get_account().m_id == first_provider.m_id) {
      REQUIRE(reply_service.get_account().m_name == first_provider.m_name);
    } else if(reply_service.get_account().m_id == second_provider.m_id) {
      REQUIRE(reply_service.get_account().m_name == second_provider.m_name);
    }
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      "new_account", "", DirectoryEntry::STAR_DIRECTORY),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      "new_account", "", DirectoryEntry::STAR_DIRECTORY),
      ServiceRequestException);
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::READ);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      "new_account", "", DirectoryEntry::STAR_DIRECTORY),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_unavailable_name") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto unavailable_name = "unavailable";
    m_data_store.make_account(unavailable_name, "",
      DirectoryEntry::STAR_DIRECTORY, second_clock::universal_time());
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      unavailable_name, "", DirectoryEntry::STAR_DIRECTORY),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_account_empty_name") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto account_name = "";
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeAccountService>(
      account_name, "", DirectoryEntry::STAR_DIRECTORY),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_create_account") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto account_name = "account";
    m_client_protocol->send_request<MakeAccountService>(
      account_name, "", DirectoryEntry::STAR_DIRECTORY);
    auto created_account = m_data_store.load_account(account_name);
    REQUIRE((created_account.m_type == DirectoryEntry::Type::ACCOUNT));
    REQUIRE(created_account.m_name == account_name);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_without_login") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      "directory", DirectoryEntry::STAR_DIRECTORY),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      "directory", DirectoryEntry::STAR_DIRECTORY), ServiceRequestException);
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::READ);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      "directory", DirectoryEntry::STAR_DIRECTORY), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "create_directory_empty_name") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto directory_name = "";
    REQUIRE_THROWS_AS(m_client_protocol->send_request<MakeDirectoryService>(
      directory_name, DirectoryEntry::STAR_DIRECTORY), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "valid_create_directory") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto directory_name = "directory";
    m_client_protocol->send_request<MakeDirectoryService>(
      directory_name, DirectoryEntry::STAR_DIRECTORY);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_entry_without_login") {
    REQUIRE_THROWS_AS(
      m_client_protocol->send_request<DeleteDirectoryEntryService>(
        DirectoryEntry::STAR_DIRECTORY), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_non_existing_entry") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto entry = DirectoryEntry::make_directory(1000, "");
    REQUIRE_THROWS_AS(
      m_client_protocol->send_request<DeleteDirectoryEntryService>(entry),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_account_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto deleted_account = DirectoryEntry::make_account(0, "deleted_account");
    deleted_account = m_data_store.make_account(deleted_account.m_name, "",
      DirectoryEntry::STAR_DIRECTORY, second_clock::universal_time());
    REQUIRE_THROWS_AS(
      m_client_protocol->send_request<DeleteDirectoryEntryService>(
        deleted_account), ServiceRequestException);
    REQUIRE(
      m_data_store.load_account(deleted_account.m_name) == deleted_account);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_account") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto deleted_account = DirectoryEntry::make_account(0, "deleted_account");
    deleted_account = m_data_store.make_account(deleted_account.m_name, "",
      DirectoryEntry::STAR_DIRECTORY, second_clock::universal_time());
    m_client_protocol->send_request<DeleteDirectoryEntryService>(
      deleted_account);
    REQUIRE_THROWS_AS(m_data_store.load_account(deleted_account.m_name),
      ServiceLocatorDataStoreException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_directory_without_permissions") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto deleted_directory =
      DirectoryEntry::make_directory(0, "deleted_directory");
    deleted_directory = m_data_store.make_directory(
      deleted_directory.m_name, DirectoryEntry::STAR_DIRECTORY);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<
      DeleteDirectoryEntryService>(deleted_directory), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "delete_directory") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::ADMINISTRATE);
    auto deleted_directory =
      DirectoryEntry::make_directory(0, "deleted_directory");
    deleted_directory = m_data_store.make_directory(
      deleted_directory.m_name, DirectoryEntry::STAR_DIRECTORY);
    m_client_protocol->send_request<DeleteDirectoryEntryService>(
      deleted_directory);
  }

  TEST_CASE_FIXTURE(Fixture, "load_directory_entry_from_path") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, Permission::READ);
    auto a = m_data_store.make_directory("a", DirectoryEntry::STAR_DIRECTORY);
    auto b = m_data_store.make_directory("b", a);
    auto c = m_data_store.make_directory("c", b);
    auto result = m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::STAR_DIRECTORY, "a");
    REQUIRE(a == result);
    result = m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::STAR_DIRECTORY, "a/b");
    REQUIRE(b == result);
    result = m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::STAR_DIRECTORY, "a/b/c");
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
      DirectoryEntry::STAR_DIRECTORY, "c"), ServiceRequestException);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoadPathService>(
      DirectoryEntry::STAR_DIRECTORY, "phantom"), ServiceRequestException);
    REQUIRE_THROWS_AS(m_client_protocol->send_request<LoadPathService>(c, ""),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_all_accounts") {
    auto account = DirectoryEntry();
    auto session_id = std::string();
    create_account_and_login(out(account), out(session_id));
    auto permissions = Permissions();
    permissions.set(Permission::ADMINISTRATE);
    permissions.set(Permission::READ);
    m_data_store.set_permissions(
      account, DirectoryEntry::STAR_DIRECTORY, permissions);
    auto a = create_user("a", "");
    auto b = create_user("b", "");
    auto c = create_user("c", "");
    auto accounts = m_client_protocol->send_request<MonitorAccountsService>();
    auto expected_accounts = std::vector<DirectoryEntry>();
    expected_accounts.push_back(DirectoryEntry::ROOT_ACCOUNT);
    expected_accounts.push_back(account);
    expected_accounts.push_back(a);
    expected_accounts.push_back(b);
    expected_accounts.push_back(c);
    REQUIRE(std::is_permutation(accounts.begin(), accounts.end(),
      expected_accounts.begin(), expected_accounts.end()));
    auto d = m_client_protocol->send_request<MakeAccountService>(
      "d", "", DirectoryEntry::STAR_DIRECTORY);
    auto update_message = std::dynamic_pointer_cast<RecordMessage<
      AccountUpdateMessage, Fixture::ClientServiceProtocolClient>>(
      m_client_protocol->read_message());
    REQUIRE(update_message);
    REQUIRE(update_message->get_record().get<0>() ==
      AccountUpdate(d, AccountUpdate::Type::ADDED));
    m_client_protocol->send_request<DeleteDirectoryEntryService>(b);
    update_message = std::dynamic_pointer_cast<RecordMessage<
      AccountUpdateMessage, Fixture::ClientServiceProtocolClient>>(
        m_client_protocol->read_message());
    REQUIRE(update_message);
    REQUIRE(update_message->get_record().get<0>() ==
      AccountUpdate(b, AccountUpdate::Type::DELETED));
  }
}
