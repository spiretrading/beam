#include <doctest/doctest.h>
#include "Beam/ServicesTests/ServiceClientFixture.hpp"
#include "Beam/ServiceLocator/ProtocolServiceLocatorClient.hpp"

using namespace Beam;
using namespace Beam::ServiceLocatorServices;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Fixture : ServiceClientFixture {
    using TestServiceLocatorClient =
      ProtocolServiceLocatorClient<TestServiceProtocolClientBuilder>;

    Fixture() {
      register_service_locator_services(out(m_server.get_slots()));
      register_service_locator_messages(out(m_server.get_slots()));
    }

    void close_server_side(TestServiceLocatorClient& client) {
      auto close_token = Async<void>();
      on_request<LocateService>([&] (auto& request, const std::string&) {
        request.set(std::vector<ServiceEntry>());
        request.get_client().close();
        close_token.get_eval().set();
      });
      try {
        client.locate("");
      } catch(const std::exception&) {}
      close_token.get();
    }

    std::unique_ptr<TestServiceLocatorClient> make_client(
        std::string username, std::string password) {
      return ServiceClientFixture::make_client<TestServiceLocatorClient>(
        std::move(username), std::move(password));
    }

    std::unique_ptr<TestServiceLocatorClient> make_client() {
      on_request<LoginService>(
        [] (auto& request, const std::string& username,
            const std::string& password) {
          auto account = DirectoryEntry::make_account(1, username);
          auto session_id = std::string("default_session");
          request.set(LoginServiceResult(account, session_id));
        });
      return make_client("test_user", "test_password");
    }
  };
}

TEST_SUITE("ProtocolServiceLocatorClient") {
  TEST_CASE("rejected_login") {
    auto fixture = Fixture();
    auto login_attempted = false;
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        REQUIRE(username == "test_user");
        REQUIRE(password == "wrong_password");
        login_attempted = true;
        request.set_exception(ServiceRequestException("Invalid credentials"));
      });
    REQUIRE_THROWS_AS(
      fixture.make_client("test_user", "wrong_password"), ConnectException);
    REQUIRE(login_attempted);
  }

  TEST_CASE("successful_login") {
    auto fixture = Fixture();
    auto login_attempted = false;
    auto expected_account = DirectoryEntry::make_account(100, "test_user");
    auto expected_session_id = std::string("session_12345");
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        REQUIRE(username == "test_user");
        REQUIRE(password == "correct_password");
        login_attempted = true;
        request.set(LoginServiceResult(expected_account, expected_session_id));
      });
    auto client = fixture.make_client("test_user", "correct_password");
    REQUIRE(login_attempted);
    REQUIRE(client->get_account() == expected_account);
    REQUIRE(client->get_session_id() == expected_session_id);
  }

  TEST_CASE("authenticate_account_insufficient_permissions") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto authentication_attempted = false;
    fixture.on_request<AuthenticateAccountService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        REQUIRE(username == "other_user");
        REQUIRE(password == "other_password");
        authentication_attempted = true;
        request.set_exception(
          ServiceRequestException("Insufficient permissions."));
      });
    REQUIRE_THROWS_AS(
      client->authenticate_account("other_user", "other_password"),
      ServiceRequestException);
    REQUIRE(authentication_attempted);
  }

  TEST_CASE("authenticate_account_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto authentication_attempted = false;
    auto expected_account = DirectoryEntry::make_account(200, "other_user");
    fixture.on_request<AuthenticateAccountService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        REQUIRE(username == "other_user");
        REQUIRE(password == "other_password");
        authentication_attempted = true;
        request.set(expected_account);
      });
    auto result = client->authenticate_account("other_user", "other_password");
    REQUIRE(authentication_attempted);
    REQUIRE(result == expected_account);
  }

  TEST_CASE("authenticate_session_invalid_session") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto authentication_attempted = false;
    auto session_id = std::string("invalid_session");
    auto key = 12345u;
    fixture.on_request<SessionAuthenticationService>(
      [&] (auto& request, const std::string& session,
          unsigned int encryption_key) {
        REQUIRE(session == session_id);
        REQUIRE(encryption_key == key);
        authentication_attempted = true;
        request.set_exception(
          ServiceRequestException("Invalid session."));
      });
    REQUIRE_THROWS_AS(
      client->authenticate_session(session_id, key),
      ServiceRequestException);
    REQUIRE(authentication_attempted);
  }

  TEST_CASE("authenticate_session_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto authentication_attempted = false;
    auto session_id = std::string("valid_session");
    auto key = 67890u;
    auto expected_account = DirectoryEntry::make_account(300, "session_user");
    fixture.on_request<SessionAuthenticationService>(
      [&] (auto& request, const std::string& session,
          unsigned int encryption_key) {
        REQUIRE(session == session_id);
        REQUIRE(encryption_key == key);
        authentication_attempted = true;
        request.set(expected_account);
      });
    auto result = client->authenticate_session(session_id, key);
    REQUIRE(authentication_attempted);
    REQUIRE(result == expected_account);
  }


  TEST_CASE("locate_service_not_found") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto locate_attempted = false;
    auto service_name = std::string("nonexistent_service");
    fixture.on_request<LocateService>(
      [&] (auto& request, const std::string& name) {
        REQUIRE(name == service_name);
        locate_attempted = true;
        request.set(std::vector<ServiceEntry>());
      });
    auto result = client->locate(service_name);
    REQUIRE(locate_attempted);
    REQUIRE(result.empty());
  }

  TEST_CASE("locate_service_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto locate_attempted = false;
    auto service_name = std::string("test_service");
    auto expected_account = DirectoryEntry::make_account(1, "service_owner");
    auto expected_properties = JsonObject();
    auto expected_service = ServiceEntry(
      service_name, expected_properties, 100, expected_account);
    fixture.on_request<LocateService>(
      [&] (auto& request, const std::string& name) {
        REQUIRE(name == service_name);
        locate_attempted = true;
        auto services = std::vector<ServiceEntry>();
        services.push_back(expected_service);
        request.set(services);
      });
    auto result = client->locate(service_name);
    REQUIRE(locate_attempted);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].get_id() == expected_service.get_id());
  }

  TEST_CASE("add_service_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto add_attempted = false;
    auto service_name = std::string("new_service");
    auto properties = JsonObject();
    properties.set("host", "localhost");
    properties.set("port", 8080);
    auto expected_account = DirectoryEntry::make_account(1, "test_user");
    auto expected_service = ServiceEntry(
      service_name, properties, 200, expected_account);
    fixture.on_request<RegisterService>(
      [&] (auto& request, const std::string& name, const JsonObject& props) {
        REQUIRE(name == service_name);
        add_attempted = true;
        request.set(expected_service);
      });
    auto result = client->add(service_name, properties);
    REQUIRE(add_attempted);
    REQUIRE(result.get_id() == expected_service.get_id());
  }

  TEST_CASE("add_service_failure") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto add_attempted = false;
    auto service_name = std::string("duplicate_service");
    auto properties = JsonObject();
    fixture.on_request<RegisterService>(
      [&] (auto& request, const std::string& name, const JsonObject& props) {
        REQUIRE(name == service_name);
        add_attempted = true;
        request.set_exception(
          ServiceRequestException("Service already exists."));
      });
    REQUIRE_THROWS_AS(
      client->add(service_name, properties), ServiceRequestException);
    REQUIRE(add_attempted);
  }

  TEST_CASE("remove_service_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto service_name = std::string("service_to_remove");
    auto properties = JsonObject();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto service = ServiceEntry(service_name, properties, 300, account);
    fixture.on_request<RegisterService>(
      [&] (auto& request, const std::string& name, const JsonObject& props) {
        request.set(service);
      });
    client->add(service_name, properties);
    auto remove_attempted = false;
    fixture.on_request<UnregisterService>(
      [&] (auto& request, int id) {
        REQUIRE(id == service.get_id());
        remove_attempted = true;
        request.set();
      });
    client->remove(service);
    REQUIRE(remove_attempted);
  }

  TEST_CASE("remove_service_not_found") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto service_name = std::string("nonexistent_service");
    auto properties = JsonObject();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto service = ServiceEntry(service_name, properties, 400, account);
    auto remove_attempted = false;
    fixture.on_request<UnregisterService>(
      [&] (auto& request, int id) {
        REQUIRE(id == service.get_id());
        remove_attempted = true;
        request.set_exception(
          ServiceRequestException("Service not found."));
      });
    REQUIRE_THROWS_AS(client->remove(service), ServiceRequestException);
    REQUIRE(remove_attempted);
  }

  TEST_CASE("load_all_accounts_empty") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    fixture.on_request<LoadAllAccountsService>(
      [&] (auto& request) {
        load_attempted = true;
        request.set(std::vector<DirectoryEntry>());
      });
    auto result = client->load_all_accounts();
    REQUIRE(load_attempted);
    REQUIRE(result.empty());
  }

  TEST_CASE("load_all_accounts_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto expected_accounts = std::vector{
      DirectoryEntry::make_account(1, "account_one"),
      DirectoryEntry::make_account(2, "account_two"),
      DirectoryEntry::make_account(3, "account_three")};
    fixture.on_request<LoadAllAccountsService>([&] (auto& request) {
      load_attempted = true;
      request.set(expected_accounts);
    });
    auto result = client->load_all_accounts();
    REQUIRE(load_attempted);
    REQUIRE(result == expected_accounts);
  }

  TEST_CASE("load_all_accounts_failure") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    fixture.on_request<LoadAllAccountsService>([&] (auto& request) {
      load_attempted = true;
      request.set_exception(
        ServiceRequestException("Insufficient permissions."));
    });
    REQUIRE_THROWS_AS(client->load_all_accounts(), ServiceRequestException);
    REQUIRE(load_attempted);
  }

  TEST_CASE("find_account_not_found") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto find_attempted = false;
    auto account_name = std::string("nonexistent_account");
    fixture.on_request<FindAccountService>(
      [&] (auto& request, const std::string& name) {
        REQUIRE(name == account_name);
        find_attempted = true;
        request.set(boost::optional<DirectoryEntry>());
      });
    auto result = client->find_account(account_name);
    REQUIRE(find_attempted);
    REQUIRE(!result.has_value());
  }

  TEST_CASE("find_account_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto find_attempted = false;
    auto account_name = std::string("existing_account");
    auto expected_account = DirectoryEntry::make_account(500, account_name);
    fixture.on_request<FindAccountService>(
      [&] (auto& request, const std::string& name) {
        REQUIRE(name == account_name);
        find_attempted = true;
        request.set(boost::optional<DirectoryEntry>(expected_account));
      });
    auto result = client->find_account(account_name);
    REQUIRE(find_attempted);
    REQUIRE(result.has_value());
    REQUIRE(result->m_id == expected_account.m_id);
    REQUIRE(result->m_name == expected_account.m_name);
  }

  TEST_CASE("make_account_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto make_attempted = false;
    auto account_name = std::string("new_account");
    auto password = std::string("secure_password");
    auto parent = DirectoryEntry::make_directory(1, "parent_directory");
    auto expected_account = DirectoryEntry::make_account(600, account_name);
    fixture.on_request<MakeAccountService>(
      [&] (auto& request, const std::string& name, const std::string& pass,
          const DirectoryEntry& parent_entry) {
        REQUIRE(name == account_name);
        REQUIRE(pass == password);
        REQUIRE(parent_entry.m_id == parent.m_id);
        make_attempted = true;
        request.set(expected_account);
      });
    auto result = client->make_account(account_name, password, parent);
    REQUIRE(make_attempted);
    REQUIRE(result.m_id == expected_account.m_id);
    REQUIRE(result.m_name == expected_account.m_name);
  }

  TEST_CASE("make_account_duplicate_name") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto make_attempted = false;
    auto account_name = std::string("duplicate_account");
    auto password = std::string("password");
    auto parent = DirectoryEntry::make_directory(1, "parent");
    fixture.on_request<MakeAccountService>(
      [&] (auto& request, const std::string& name, const std::string& pass,
          const DirectoryEntry& parent_entry) {
        REQUIRE(name == account_name);
        make_attempted = true;
        request.set_exception(
          ServiceRequestException("Account already exists."));
      });
    REQUIRE_THROWS_AS(client->make_account(account_name, password, parent),
      ServiceRequestException);
    REQUIRE(make_attempted);
  }

  TEST_CASE("make_directory_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto make_attempted = false;
    auto directory_name = std::string("new_directory");
    auto parent = DirectoryEntry::make_directory(1, "root");
    auto expected_directory =
      DirectoryEntry::make_directory(700, directory_name);
    fixture.on_request<MakeDirectoryService>(
      [&] (auto& request, const std::string& name,
          const DirectoryEntry& parent_entry) {
        REQUIRE(name == directory_name);
        REQUIRE(parent_entry.m_id == parent.m_id);
        make_attempted = true;
        request.set(expected_directory);
      });
    auto result = client->make_directory(directory_name, parent);
    REQUIRE(make_attempted);
    REQUIRE(result.m_id == expected_directory.m_id);
    REQUIRE(result.m_name == expected_directory.m_name);
  }

  TEST_CASE("make_directory_insufficient_permissions") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto make_attempted = false;
    auto directory_name = std::string("restricted_directory");
    auto parent = DirectoryEntry::make_directory(1, "root");
    fixture.on_request<MakeDirectoryService>(
      [&] (auto& request, const std::string& name,
          const DirectoryEntry& parent_entry) {
        REQUIRE(name == directory_name);
        make_attempted = true;
        request.set_exception(
          ServiceRequestException("Insufficient permissions."));
      });
    REQUIRE_THROWS_AS(
      client->make_directory(directory_name, parent), ServiceRequestException);
    REQUIRE(make_attempted);
  }

  TEST_CASE("store_password_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto store_attempted = false;
    auto account = DirectoryEntry::make_account(800, "target_account");
    auto password = std::string("new_secure_password");
    fixture.on_request<StorePasswordService>(
      [&] (auto& request, const DirectoryEntry& target_account,
          const std::string& pass) {
        REQUIRE(target_account.m_id == account.m_id);
        REQUIRE(pass == password);
        store_attempted = true;
        request.set();
      });
    client->store_password(account, password);
    REQUIRE(store_attempted);
  }

  TEST_CASE("store_password_insufficient_permissions") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto store_attempted = false;
    auto account = DirectoryEntry::make_account(900, "other_account");
    auto password = std::string("password");
    fixture.on_request<StorePasswordService>(
      [&] (auto& request, const DirectoryEntry& target_account,
          const std::string& pass) {
        REQUIRE(target_account.m_id == account.m_id);
        store_attempted = true;
        request.set_exception(
          ServiceRequestException("Insufficient permissions."));
      });
    REQUIRE_THROWS_AS(
      client->store_password(account, password), ServiceRequestException);
    REQUIRE(store_attempted);
  }

  TEST_CASE("load_directory_entry_by_path_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto root = DirectoryEntry::make_directory(1, "root");
    auto path = std::string("users/admin");
    auto expected_entry = DirectoryEntry::make_account(1000, "admin");
    fixture.on_request<LoadPathService>(
      [&] (auto& request, const DirectoryEntry& root_entry,
          const std::string& entry_path) {
        REQUIRE(root_entry.m_id == root.m_id);
        REQUIRE(entry_path == path);
        load_attempted = true;
        request.set(expected_entry);
      });
    auto result = client->load_directory_entry(root, path);
    REQUIRE(load_attempted);
    REQUIRE(result.m_id == expected_entry.m_id);
    REQUIRE(result.m_name == expected_entry.m_name);
  }

  TEST_CASE("load_directory_entry_by_path_not_found") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto root = DirectoryEntry::make_directory(1, "root");
    auto path = std::string("nonexistent/path");
    fixture.on_request<LoadPathService>(
      [&] (auto& request, const DirectoryEntry& root_entry,
          const std::string& entry_path) {
        REQUIRE(root_entry.m_id == root.m_id);
        REQUIRE(entry_path == path);
        load_attempted = true;
        request.set_exception(ServiceRequestException("Path not found."));
      });
    REQUIRE_THROWS_AS(
      client->load_directory_entry(root, path), ServiceRequestException);
    REQUIRE(load_attempted);
  }

  TEST_CASE("load_directory_entry_by_id_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto entry_id = 1100u;
    auto expected_entry = DirectoryEntry::make_account(entry_id, "user");
    fixture.on_request<LoadDirectoryEntryService>(
      [&] (auto& request, unsigned int id) {
        REQUIRE(id == entry_id);
        load_attempted = true;
        request.set(expected_entry);
      });
    auto result = client->load_directory_entry(entry_id);
    REQUIRE(load_attempted);
    REQUIRE(result.m_id == expected_entry.m_id);
    REQUIRE(result.m_name == expected_entry.m_name);
  }

  TEST_CASE("load_directory_entry_by_id_not_found") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto entry_id = 9999u;
    fixture.on_request<LoadDirectoryEntryService>(
      [&] (auto& request, unsigned int id) {
        REQUIRE(id == entry_id);
        load_attempted = true;
        request.set_exception(
          ServiceRequestException("Directory entry not found."));
      });
    REQUIRE_THROWS_AS(
      client->load_directory_entry(entry_id), ServiceRequestException);
    REQUIRE(load_attempted);
  }

  TEST_CASE("load_parents_empty") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto entry = DirectoryEntry::make_directory(1, "root");
    fixture.on_request<LoadParentsService>(
      [&] (auto& request, const DirectoryEntry& target_entry) {
        REQUIRE(target_entry.m_id == entry.m_id);
        load_attempted = true;
        request.set(std::vector<DirectoryEntry>());
      });
    auto result = client->load_parents(entry);
    REQUIRE(load_attempted);
    REQUIRE(result.empty());
  }

  TEST_CASE("load_parents_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto entry = DirectoryEntry::make_account(1200, "child_account");
    auto expected_parents = std::vector{
      DirectoryEntry::make_directory(1, "parent_one"),
      DirectoryEntry::make_directory(2, "parent_two")};
    fixture.on_request<LoadParentsService>(
      [&] (auto& request, const DirectoryEntry& target_entry) {
        REQUIRE(target_entry.m_id == entry.m_id);
        load_attempted = true;
        request.set(expected_parents);
      });
    auto result = client->load_parents(entry);
    REQUIRE(load_attempted);
    REQUIRE(result == expected_parents);
  }

  TEST_CASE("load_children_empty") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto entry = DirectoryEntry::make_directory(1300, "empty_directory");
    fixture.on_request<LoadChildrenService>(
      [&] (auto& request, const DirectoryEntry& target_entry) {
        REQUIRE(target_entry.m_id == entry.m_id);
        load_attempted = true;
        request.set(std::vector<DirectoryEntry>());
      });
    auto result = client->load_children(entry);
    REQUIRE(load_attempted);
    REQUIRE(result.empty());
  }

  TEST_CASE("load_children_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto entry = DirectoryEntry::make_directory(1400, "parent_directory");
    auto expected_children = std::vector{
      DirectoryEntry::make_account(10, "child_one"),
      DirectoryEntry::make_directory(20, "child_two"),
      DirectoryEntry::make_account(30, "child_three")};
    fixture.on_request<LoadChildrenService>(
      [&] (auto& request, const DirectoryEntry& target_entry) {
        REQUIRE(target_entry.m_id == entry.m_id);
        load_attempted = true;
        request.set(expected_children);
      });
    auto result = client->load_children(entry);
    REQUIRE(load_attempted);
    REQUIRE(result == expected_children);
  }

  TEST_CASE("remove_directory_entry_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto remove_attempted = false;
    auto entry = DirectoryEntry::make_directory(1500, "directory_to_remove");
    fixture.on_request<DeleteDirectoryEntryService>(
      [&] (auto& request, const DirectoryEntry& target_entry) {
        REQUIRE(target_entry.m_id == entry.m_id);
        remove_attempted = true;
        request.set();
      });
    client->remove(entry);
    REQUIRE(remove_attempted);
  }

  TEST_CASE("remove_directory_entry_not_empty") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto remove_attempted = false;
    auto entry = DirectoryEntry::make_directory(1600, "nonempty_directory");
    fixture.on_request<DeleteDirectoryEntryService>(
      [&] (auto& request, const DirectoryEntry& target_entry) {
        REQUIRE(target_entry.m_id == entry.m_id);
        remove_attempted = true;
        request.set_exception(
          ServiceRequestException("Directory is not empty."));
      });
    REQUIRE_THROWS_AS(client->remove(entry), ServiceRequestException);
    REQUIRE(remove_attempted);
  }

  TEST_CASE("associate_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto associate_attempted = false;
    auto entry = DirectoryEntry::make_account(1700, "child_entry");
    auto parent = DirectoryEntry::make_directory(1800, "parent_entry");
    fixture.on_request<AssociateService>(
      [&] (auto& request, const DirectoryEntry& child,
          const DirectoryEntry& parent_entry) {
        REQUIRE(child.m_id == entry.m_id);
        REQUIRE(parent_entry.m_id == parent.m_id);
        associate_attempted = true;
        request.set();
      });
    client->associate(entry, parent);
    REQUIRE(associate_attempted);
  }

  TEST_CASE("associate_insufficient_permissions") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto associate_attempted = false;
    auto entry = DirectoryEntry::make_account(1900, "entry");
    auto parent = DirectoryEntry::make_directory(2000, "restricted_parent");
    fixture.on_request<AssociateService>(
      [&] (auto& request, const DirectoryEntry& child,
          const DirectoryEntry& parent_entry) {
        REQUIRE(child.m_id == entry.m_id);
        REQUIRE(parent_entry.m_id == parent.m_id);
        associate_attempted = true;
        request.set_exception(
          ServiceRequestException("Insufficient permissions."));
      });
    REQUIRE_THROWS_AS(
      client->associate(entry, parent), ServiceRequestException);
    REQUIRE(associate_attempted);
  }

  TEST_CASE("detach_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto detach_attempted = false;
    auto entry = DirectoryEntry::make_account(2100, "child_entry");
    auto parent = DirectoryEntry::make_directory(2200, "parent_entry");
    fixture.on_request<DetachService>(
      [&] (auto& request, const DirectoryEntry& child,
          const DirectoryEntry& parent_entry) {
        REQUIRE(child.m_id == entry.m_id);
        REQUIRE(parent_entry.m_id == parent.m_id);
        detach_attempted = true;
        request.set();
      });
    client->detach(entry, parent);
    REQUIRE(detach_attempted);
  }

  TEST_CASE("detach_not_associated") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto detach_attempted = false;
    auto entry = DirectoryEntry::make_account(2300, "entry");
    auto parent = DirectoryEntry::make_directory(2400, "parent");
    fixture.on_request<DetachService>(
      [&] (auto& request, const DirectoryEntry& child,
          const DirectoryEntry& parent_entry) {
        REQUIRE(child.m_id == entry.m_id);
        REQUIRE(parent_entry.m_id == parent.m_id);
        detach_attempted = true;
        request.set_exception(
          ServiceRequestException("Entry is not associated with parent."));
      });
    REQUIRE_THROWS_AS(client->detach(entry, parent), ServiceRequestException);
    REQUIRE(detach_attempted);
  }

  TEST_CASE("has_permissions_true") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto check_attempted = false;
    auto account = DirectoryEntry::make_account(2500, "account");
    auto target = DirectoryEntry::make_directory(2600, "target");
    auto permissions = Permission::READ;
    fixture.on_request<HasPermissionsService>(
      [&] (auto& request, const DirectoryEntry& source,
          const DirectoryEntry& target_entry, Permissions perms) {
        REQUIRE(source.m_id == account.m_id);
        REQUIRE(target_entry.m_id == target.m_id);
        REQUIRE(perms == permissions);
        check_attempted = true;
        request.set(true);
      });
    auto result = client->has_permissions(account, target, permissions);
    REQUIRE(check_attempted);
    REQUIRE(result);
  }

  TEST_CASE("has_permissions_false") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto check_attempted = false;
    auto account = DirectoryEntry::make_account(2700, "account");
    auto target = DirectoryEntry::make_directory(2800, "target");
    auto permissions = Permission::ADMINISTRATE;
    fixture.on_request<HasPermissionsService>(
      [&] (auto& request, const DirectoryEntry& source,
          const DirectoryEntry& target_entry, Permissions perms) {
        REQUIRE(source.m_id == account.m_id);
        REQUIRE(target_entry.m_id == target.m_id);
        REQUIRE(perms == permissions);
        check_attempted = true;
        request.set(false);
      });
    auto result = client->has_permissions(account, target, permissions);
    REQUIRE(check_attempted);
    REQUIRE(!result);
  }

  TEST_CASE("store_permissions_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto store_attempted = false;
    auto source = DirectoryEntry::make_account(2900, "source");
    auto target = DirectoryEntry::make_directory(3000, "target");
    auto permissions = Permissions().
      set(Permission::READ).
      set(Permission::ADMINISTRATE);
    fixture.on_request<StorePermissionsService>(
      [&] (auto& request, const DirectoryEntry& source_entry,
          const DirectoryEntry& target_entry, Permissions perms) {
        REQUIRE(source_entry.m_id == source.m_id);
        REQUIRE(target_entry.m_id == target.m_id);
        REQUIRE(perms == permissions);
        store_attempted = true;
        request.set();
      });
    client->store(source, target, permissions);
    REQUIRE(store_attempted);
  }

  TEST_CASE("store_permissions_insufficient_permissions") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto store_attempted = false;
    auto source = DirectoryEntry::make_account(3100, "source");
    auto target = DirectoryEntry::make_directory(3200, "target");
    auto permissions = Permission::ADMINISTRATE;
    fixture.on_request<StorePermissionsService>(
      [&] (auto& request, const DirectoryEntry& source_entry,
          const DirectoryEntry& target_entry, Permissions perms) {
        REQUIRE(source_entry.m_id == source.m_id);
        REQUIRE(target_entry.m_id == target.m_id);
        store_attempted = true;
        request.set_exception(
          ServiceRequestException("Insufficient permissions."));
      });
    REQUIRE_THROWS_AS(
      client->store(source, target, permissions), ServiceRequestException);
    REQUIRE(store_attempted);
  }

  TEST_CASE("load_registration_time_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto account = DirectoryEntry::make_account(3300, "account");
    auto expected_time = time_from_string("2023-10-15 14:30:00");
    fixture.on_request<LoadRegistrationTimeService>(
      [&] (auto& request, const DirectoryEntry& target_account) {
        REQUIRE(target_account.m_id == account.m_id);
        load_attempted = true;
        request.set(expected_time);
      });
    auto result = client->load_registration_time(account);
    REQUIRE(load_attempted);
    REQUIRE(result == expected_time);
  }

  TEST_CASE("load_registration_time_account_not_found") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto account = DirectoryEntry::make_account(3400, "nonexistent_account");
    fixture.on_request<LoadRegistrationTimeService>(
      [&] (auto& request, const DirectoryEntry& target_account) {
        REQUIRE(target_account.m_id == account.m_id);
        load_attempted = true;
        request.set_exception(ServiceRequestException("Account not found."));
      });
    REQUIRE_THROWS_AS(
      client->load_registration_time(account), ServiceRequestException);
    REQUIRE(load_attempted);
  }

  TEST_CASE("load_last_login_time_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto account = DirectoryEntry::make_account(3500, "account");
    auto expected_time = time_from_string("2023-10-17 09:15:00");
    fixture.on_request<LoadLastLoginTimeService>(
      [&] (auto& request, const DirectoryEntry& target_account) {
        REQUIRE(target_account.m_id == account.m_id);
        load_attempted = true;
        request.set(expected_time);
      });
    auto result = client->load_last_login_time(account);
    REQUIRE(load_attempted);
    REQUIRE(result == expected_time);
  }

  TEST_CASE("load_last_login_time_never_logged_in") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto load_attempted = false;
    auto account = DirectoryEntry::make_account(3600, "new_account");
    auto expected_time = not_a_date_time;
    fixture.on_request<LoadLastLoginTimeService>(
      [&] (auto& request, const DirectoryEntry& target_account) {
        REQUIRE(target_account.m_id == account.m_id);
        load_attempted = true;
        request.set(expected_time);
      });
    auto result = client->load_last_login_time(account);
    REQUIRE(load_attempted);
    REQUIRE(result.is_not_a_date_time());
  }

  TEST_CASE("rename_successful") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto rename_attempted = false;
    auto entry = DirectoryEntry::make_account(3700, "old_name");
    auto new_name = std::string("new_name");
    auto expected_entry = DirectoryEntry::make_account(3700, new_name);
    fixture.on_request<RenameService>(
      [&] (auto& request, const DirectoryEntry& target_entry,
          const std::string& name) {
        REQUIRE(target_entry.m_id == entry.m_id);
        REQUIRE(name == new_name);
        rename_attempted = true;
        request.set(expected_entry);
      });
    auto result = client->rename(entry, new_name);
    REQUIRE(rename_attempted);
    REQUIRE(result.m_id == expected_entry.m_id);
    REQUIRE(result.m_name == expected_entry.m_name);
  }

  TEST_CASE("rename_duplicate_name") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto rename_attempted = false;
    auto entry = DirectoryEntry::make_account(3800, "account");
    auto new_name = std::string("duplicate_name");
    fixture.on_request<RenameService>(
      [&] (auto& request, const DirectoryEntry& target_entry,
          const std::string& name) {
        REQUIRE(target_entry.m_id == entry.m_id);
        REQUIRE(name == new_name);
        rename_attempted = true;
        request.set_exception(ServiceRequestException("Name already exists."));
      });
    REQUIRE_THROWS_AS(client->rename(entry, new_name), ServiceRequestException);
    REQUIRE(rename_attempted);
  }

  TEST_CASE("monitor_accounts") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto unmonitor_token = Async<void>();
    auto test_accounts = std::vector{
      DirectoryEntry::make_account(123, "account_a"),
      DirectoryEntry::make_account(124, "account_b"),
      DirectoryEntry::make_account(125, "account_c")};
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<MonitorAccountsService>([&] (auto& request) {
      server_client = &request.get_client();
      request.set(test_accounts);
    });
    fixture.on_request<UnmonitorAccountsService>([&] (auto& request) {
      request.set();
      unmonitor_token.get_eval().set();
    });
    auto queue = std::make_shared<Queue<AccountUpdate>>();
    client->monitor(queue);
    auto update = queue->pop();
    REQUIRE(update.m_account == test_accounts[0]);
    REQUIRE(update.m_type == AccountUpdate::Type::ADDED);
    update = queue->pop();
    REQUIRE(update.m_account == test_accounts[1]);
    REQUIRE(update.m_type == AccountUpdate::Type::ADDED);
    update = queue->pop();
    REQUIRE(update.m_account == test_accounts[2]);
    REQUIRE(update.m_type == AccountUpdate::Type::ADDED);
    REQUIRE(server_client);
    send_record_message<AccountUpdateMessage>(
      *server_client, AccountUpdate::remove(test_accounts[0]));
    update = queue->pop();
    REQUIRE(update.m_account == test_accounts[0]);
    REQUIRE(update.m_type == AccountUpdate::Type::DELETED);
    auto duplicate_queue = std::make_shared<Queue<AccountUpdate>>();
    client->monitor(duplicate_queue);
    update = duplicate_queue->pop();
    REQUIRE(update.m_account == test_accounts[1]);
    REQUIRE(update.m_type == AccountUpdate::Type::ADDED);
    update = duplicate_queue->pop();
    REQUIRE(update.m_account == test_accounts[2]);
    REQUIRE(update.m_type == AccountUpdate::Type::ADDED);
    queue->close();
    duplicate_queue->close();
    send_record_message<AccountUpdateMessage>(
      *server_client, AccountUpdate::remove(test_accounts[1]));
    REQUIRE_NOTHROW(unmonitor_token.get());
  }

  TEST_CASE("monitor_accounts_reconnect") {
    auto fixture = Fixture();
    auto reconnect_count = 0;
    auto test_accounts = std::vector{
      DirectoryEntry::make_account(123, "account_a"),
      DirectoryEntry::make_account(124, "account_b"),
      DirectoryEntry::make_account(125, "account_c")};
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        auto account = DirectoryEntry::make_account(1, username);
        auto session_id = std::string("session");
        ++reconnect_count;
        request.set(LoginServiceResult(account, session_id));
      });
    fixture.on_request<MonitorAccountsService>([&] (auto& request) {
      if(reconnect_count > 1) {
        auto updated_accounts = test_accounts;
        updated_accounts.push_back(
          DirectoryEntry::make_account(135, "account_d"));
        request.set(updated_accounts);
      } else {
        request.set(test_accounts);
      }
    });
    auto client = fixture.make_client("test_user", "test_password");
    auto queue = std::make_shared<Queue<AccountUpdate>>();
    client->monitor(queue);
    for(auto i = std::size_t(0); i != test_accounts.size(); ++i) {
      queue->pop();
    }
    fixture.close_server_side(*client);
    auto update = queue->pop();
    REQUIRE(update.m_account.m_id == 135);
    REQUIRE(update.m_account.m_name == "account_d");
    REQUIRE(update.m_type == AccountUpdate::Type::ADDED);
    client->close();
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
  }

  TEST_CASE("register_service_reconnect") {
    auto fixture = Fixture();
    auto reconnect_count = 0;
    auto next_id = 1;
    auto registered_services = std::vector<ServiceEntry>();
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        auto account = DirectoryEntry::make_account(1, username);
        auto session_id = std::string("session");
        ++reconnect_count;
        request.set(LoginServiceResult(account, session_id));
      });
    auto recovery_token = Async<void>();
    fixture.on_request<RegisterService>(
      [&] (auto& request, const std::string& name,
          const JsonObject& properties) {
        ++next_id;
        auto service = ServiceEntry(name, properties, next_id,
          DirectoryEntry::make_account(12, "service"));
        registered_services.push_back(service);
        request.set(service);
        if(next_id == 5) {
          recovery_token.get_eval().set();
        }
      });
    auto client = fixture.make_client("test_user", "test_password");
    auto properties_one = JsonObject();
    properties_one.set("meta1", 12);
    properties_one.set("meta2", "alpha");
    auto service_one = client->add("service_one", properties_one);
    auto properties_two = JsonObject();
    properties_two.set("meta3", "beta");
    properties_two.set("meta4", false);
    auto service_two = client->add("service_two", properties_two);
    auto original_count = registered_services.size();
    registered_services.clear();
    fixture.close_server_side(*client);
    REQUIRE_NOTHROW(recovery_token.get());
    REQUIRE(reconnect_count == 2);
    REQUIRE(registered_services.size() == 2);
    REQUIRE(registered_services[0].get_account() == service_one.get_account());
    REQUIRE(registered_services[0].get_name() == service_one.get_name());
    REQUIRE(
      registered_services[0].get_properties() == service_one.get_properties());
    REQUIRE(registered_services[1].get_account() == service_two.get_account());
    REQUIRE(registered_services[1].get_name() == service_two.get_name());
    REQUIRE(
      registered_services[1].get_properties() == service_two.get_properties());
  }
}
