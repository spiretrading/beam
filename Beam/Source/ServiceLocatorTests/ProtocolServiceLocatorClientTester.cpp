#include <atomic>
#include <boost/scope/scope_exit.hpp>
#include <doctest/doctest.h>
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/ServicesTests/ServiceClientFixture.hpp"
#include "Beam/ServiceLocator/ProtocolServiceLocatorClient.hpp"

using namespace Beam;
using namespace Beam::ServiceLocatorServices;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct TrackingClientBuilder : TestServiceProtocolClientBuilder {
    ScopedQueueWriter<std::shared_ptr<Client>> m_clients;

    TrackingClientBuilder(TestServiceProtocolClientBuilder builder,
        ScopedQueueWriter<std::shared_ptr<Client>> clients)
        : TestServiceProtocolClientBuilder(std::move(builder)),
          m_clients(std::move(clients)) {}

    std::shared_ptr<Client> make_client(const ServiceSlots<Client>& slots) {
      auto client = std::shared_ptr<Client>(
        TestServiceProtocolClientBuilder::make_client(slots));
      m_clients.push(client);
      return client;
    }
  };

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

    std::unique_ptr<TestServiceLocatorClient> make_session_client(
        const std::string& session_id, unsigned int key) {
      return ServiceClientFixture::make_client<TestServiceLocatorClient>(
        session_id, key);
    }
  };
}

TEST_SUITE("ProtocolServiceLocatorClient") {
  TEST_CASE("close_during_reconnect") {
    auto fixture = Fixture();
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        server_client = &request.get_client();
        request.set(LoginServiceResult(
          DirectoryEntry::make_account(1, username), "session"));
      });
    auto connection_attempts = std::atomic_int();
    auto is_available = std::atomic_bool(true);
    auto builder = TestServiceProtocolClientBuilder([&] {
      ++connection_attempts;
      if(!is_available) {
        throw ConnectException("Unavailable.");
      }
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
        "test", *fixture.m_server_connection);
    }, [] {
      return std::make_unique<TriggerTimer>();
    });
    auto client = std::make_unique<Fixture::TestServiceLocatorClient>(
      "user", "password", builder);
    is_available = false;
    server_client->close();
    flush_pending_routines();
    auto tasks = RoutineHandlerGroup();
    auto queue = std::make_shared<Queue<AccountUpdate>>();
    auto services = std::make_shared<Queue<ServiceUpdate>>();
    SUBCASE("lookup") {
      for(auto i = 0; i != 2; ++i) {
        tasks.spawn([&] {
          REQUIRE_THROWS_AS(client->locate("market_data_relay_service"),
            IOException);
        });
      }
    }
    SUBCASE("monitor") {
      client->monitor(queue);
    }
    SUBCASE("monitor_services") {
      client->monitor("market_data_service", services);
    }
    flush_pending_routines();
    auto attempts = connection_attempts.load();
    client->close();
    tasks.wait();
    flush_pending_routines();
    REQUIRE(connection_attempts == attempts);
    REQUIRE_THROWS_AS(client->locate("market_data_relay_service"),
      IOException);
    REQUIRE(connection_attempts == attempts);
  }

  TEST_CASE("rejected_login") {
    auto fixture = Fixture();
    auto login_attempted = false;
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string& username,
          const std::string& password) {
        REQUIRE(username == "test_user");
        REQUIRE(password == "wrong_password");
        login_attempted = true;
        request.set_exception(
          ServiceRequestException("Invalid username or password."));
      });
    REQUIRE_THROWS_AS(
      fixture.make_client("test_user", "wrong_password"),
      AuthenticationException);
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

  TEST_CASE("monitor_services") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::make_account(12, "provider");
    auto first = ServiceEntry("quotes", JsonObject(), 1, account);
    auto second = ServiceEntry("quotes", JsonObject(), 2, account);
    auto other = ServiceEntry("orders", JsonObject(), 3, account);
    auto requests = 0;
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string& name) {
        ++requests;
        server_client = &request.get_client();
        if(name == "quotes") {
          send_record_message<ServiceAvailabilityMessage>(
            request.get_client(), second, true);
          send_record_message<ServiceAvailabilityMessage>(
            request.get_client(), first, false);
          request.set(std::vector{first});
        } else {
          REQUIRE(name == "orders");
          request.set(std::vector{other});
        }
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(first));
    REQUIRE(queue->pop() == ServiceUpdate::add(second));
    REQUIRE(queue->pop() == ServiceUpdate::remove(first));
    auto duplicate = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", duplicate);
    REQUIRE(duplicate->pop() == ServiceUpdate::add(second));
    REQUIRE(requests == 1);
    auto separate = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("orders", separate);
    REQUIRE(separate->pop() == ServiceUpdate::add(other));
    REQUIRE(requests == 2);
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, second, true);
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, first, false);
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, second, false);
    REQUIRE(queue->pop() == ServiceUpdate::remove(second));
    REQUIRE(duplicate->pop() == ServiceUpdate::remove(second));
    REQUIRE(!separate->try_pop());
    queue->close();
    duplicate->close();
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, first, true);
    flush_pending_routines();
    auto replacement = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", replacement);
    REQUIRE(replacement->pop() == ServiceUpdate::add(first));
    REQUIRE(requests == 2);
    client->close();
    REQUIRE_THROWS_AS(replacement->pop(), PipeBrokenException);
    REQUIRE_THROWS_AS(separate->pop(), PipeBrokenException);
  }

  TEST_CASE("monitor_services_empty_snapshot") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto service = ServiceEntry("quotes", JsonObject(), 1,
      DirectoryEntry::make_account(12, "provider"));
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string& name) {
        REQUIRE(name == "quotes");
        request.set(std::vector<ServiceEntry>());
        send_record_message<ServiceAvailabilityMessage>(
          request.get_client(), service, true);
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
  }

  TEST_CASE("monitor_services_reconnect") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::make_account(12, "provider");
    auto first = ServiceEntry("quotes", JsonObject(), 1, account);
    auto second = ServiceEntry("quotes", JsonObject(), 2, account);
    auto third = ServiceEntry("quotes", JsonObject(), 3, account);
    auto fourth = ServiceEntry("quotes", JsonObject(), 4, account);
    auto properties = JsonObject();
    properties.set("scope", "new_scope");
    auto replacement = ServiceEntry("quotes", properties, 2, account);
    SUBCASE("properties_changed") {}
    SUBCASE("account_changed") {
      replacement = ServiceEntry("quotes", JsonObject(), 2,
        DirectoryEntry::make_account(13, "replacement"));
    }
    SUBCASE("account_renamed") {
      replacement = ServiceEntry("quotes", JsonObject(), 2,
        DirectoryEntry::make_account(12, "renamed"));
    }
    auto requests = 0;
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string& name) {
        REQUIRE(name == "quotes");
        ++requests;
        if(requests == 1) {
          request.set(std::vector{first, second, third});
        } else {
          request.set(std::vector{replacement, third, fourth});
          send_record_message<ServiceAvailabilityMessage>(
            request.get_client(), third, false);
        }
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(first));
    REQUIRE(queue->pop() == ServiceUpdate::add(second));
    REQUIRE(queue->pop() == ServiceUpdate::add(third));
    fixture.close_server_side(*client);
    REQUIRE(queue->pop() == ServiceUpdate::remove(first));
    REQUIRE(queue->pop() == ServiceUpdate::remove(second));
    auto update = queue->pop();
    REQUIRE(update == ServiceUpdate::add(replacement));
    REQUIRE(update.m_service.get_properties() == replacement.get_properties());
    REQUIRE(update.m_service.get_account() == replacement.get_account());
    REQUIRE(update.m_service.get_account().m_name ==
      replacement.get_account().m_name);
    REQUIRE(queue->pop() == ServiceUpdate::add(fourth));
    REQUIRE(queue->pop() == ServiceUpdate::remove(third));
    REQUIRE(requests == 2);
    auto duplicate = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", duplicate);
    REQUIRE(duplicate->pop() == ServiceUpdate::add(replacement));
    REQUIRE(duplicate->pop() == ServiceUpdate::add(fourth));
    REQUIRE(requests == 2);
    client->close();
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
    REQUIRE_THROWS_AS(duplicate->pop(), PipeBrokenException);
  }

  TEST_CASE("monitor_services_replacement") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto service = ServiceEntry("quotes", JsonObject(), 1,
      DirectoryEntry::make_account(12, "provider"));
    auto properties = JsonObject();
    properties.set("scope", "TSX");
    auto replacement = ServiceEntry("quotes", properties, 1,
      service.get_account());
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        server_client = &request.get_client();
        request.set(std::vector{service});
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, replacement, true);
    REQUIRE(queue->pop() == ServiceUpdate::remove(service));
    REQUIRE(queue->pop() == ServiceUpdate::add(replacement));
    auto duplicate = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", duplicate);
    REQUIRE(duplicate->pop() == ServiceUpdate::add(replacement));
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, service, false);
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, replacement, false);
    REQUIRE(queue->pop() == ServiceUpdate::remove(replacement));
    REQUIRE(duplicate->pop() == ServiceUpdate::remove(replacement));
    client->close();
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
    REQUIRE_THROWS_AS(duplicate->pop(), PipeBrokenException);
  }

  TEST_CASE("monitor_services_stale_connection") {
    auto fixture = Fixture();
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string&, const std::string&) {
        request.set(LoginServiceResult(
          DirectoryEntry::make_account(1, "subscriber"), "session"));
      });
    auto service = ServiceEntry("quotes", JsonObject(), 1,
      DirectoryEntry::make_account(12, "provider"));
    auto marker = ServiceEntry("quotes", JsonObject(), 2,
      service.get_account());
    auto requests = 0;
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        ++requests;
        request.set(std::vector{service});
        if(requests == 2) {
          send_record_message<ServiceAvailabilityMessage>(
            request.get_client(), marker, true);
        }
      });
    auto connections = std::make_shared<Queue<
      std::shared_ptr<TrackingClientBuilder::Client>>>();
    auto subscriber = std::make_unique<
      ProtocolServiceLocatorClient<TrackingClientBuilder>>(
        "subscriber", "password", TrackingClientBuilder(
          TestServiceProtocolClientBuilder([&] {
            return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
              "subscriber", *fixture.m_server_connection);
          }, [] {
            return std::make_unique<TriggerTimer>();
          }), connections));
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    subscriber->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
    auto previous = connections->pop();
    previous->close();
    auto current = connections->pop();
    REQUIRE(queue->pop() == ServiceUpdate::add(marker));
    auto stale = RecordMessage<
      ServiceAvailabilityMessage, TrackingClientBuilder::Client>(
        service, false);
    stale.emit(previous->get_slots().find(stale), Ref(*previous));
    auto reference = std::weak_ptr(previous);
    previous.reset();
    REQUIRE(reference.expired());
    auto removal = RecordMessage<
      ServiceAvailabilityMessage, TrackingClientBuilder::Client>(marker, false);
    removal.emit(current->get_slots().find(removal), Ref(*current));
    REQUIRE(queue->pop() == ServiceUpdate::remove(marker));
    auto duplicate = std::make_shared<Queue<ServiceUpdate>>();
    subscriber->monitor("quotes", duplicate);
    REQUIRE(duplicate->pop() == ServiceUpdate::add(service));
    REQUIRE(requests == 2);
    subscriber->close();
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
  }

  TEST_CASE("monitor_services_interrupted_subscription") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto requests = 0;
    auto service = ServiceEntry("quotes", JsonObject(), 1,
      DirectoryEntry::make_account(12, "provider"));
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string& name) {
        ++requests;
        if(requests == 1) {
          request.get_client().close();
        } else {
          request.set(std::vector{service});
        }
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
    REQUIRE(requests == 2);
  }

  TEST_CASE("monitor_services_rejected_subscription") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string& name) {
        request.set_exception(ServiceRequestException("Denied."));
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE_THROWS_AS(queue->pop(), ServiceRequestException);
  }

  TEST_CASE("monitor_services_resubscribe") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto service = ServiceEntry("quotes", JsonObject(), 1,
      DirectoryEntry::make_account(12, "provider"));
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    auto snapshot = std::vector{service};
    auto is_subscribed = false;
    auto requests = 0;
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        server_client = &request.get_client();
        ++requests;
        is_subscribed = true;
        request.set(snapshot);
      });
    auto unsubscribe = Async<void>();
    auto resume = Async<void>();
    fixture.on_request<UnsubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        unsubscribe.get();
        is_subscribed = false;
        resume.get();
        request.set();
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
    queue->close();
    snapshot.clear();
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, service, false);
    flush_pending_routines();
    auto replacement = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", replacement);
    snapshot.push_back(service);
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, service, true);
    flush_pending_routines();
    unsubscribe.get_eval().set();
    flush_pending_routines();
    snapshot.clear();
    if(is_subscribed) {
      send_record_message<ServiceAvailabilityMessage>(
        *server_client, service, false);
    }
    resume.get_eval().set();
    flush_pending_routines();
    auto entries = std::vector<ServiceEntry>();
    while(auto update = replacement->try_pop()) {
      if(update->m_type == ServiceUpdate::Type::ADDED) {
        entries.push_back(update->m_service);
      } else {
        std::erase(entries, update->m_service);
      }
    }
    REQUIRE(entries.empty());
    auto marker = ServiceEntry("quotes", JsonObject(), 2,
      service.get_account());
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, marker, true);
    flush_pending_routines();
    REQUIRE((replacement->try_pop() == ServiceUpdate::add(marker)));
    replacement->close();
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, marker, false);
    flush_pending_routines();
    fixture.close_server_side(*client);
    flush_pending_routines();
    REQUIRE(requests == 1);
  }

  TEST_CASE("monitor_services_reconnect_recovery_failure") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto service = ServiceEntry("quotes", JsonObject(), 1,
      DirectoryEntry::make_account(12, "provider"));
    auto marker = ServiceEntry("quotes", JsonObject(), 2,
      service.get_account());
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        server_client = &request.get_client();
        request.set(std::vector{service});
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
    auto accounts = std::make_shared<Queue<AccountUpdate>>();
    auto requests = 0;
    auto disconnect = false;
    auto monitor_accounts = false;
    SUBCASE("registration_rejected") {}
    SUBCASE("registration_interrupted") {
      disconnect = true;
    }
    SUBCASE("accounts_rejected") {
      monitor_accounts = true;
    }
    SUBCASE("accounts_interrupted") {
      monitor_accounts = true;
      disconnect = true;
    }
    if(monitor_accounts) {
      fixture.on_request<MonitorAccountsService>([&] (auto& request) {
        ++requests;
        if(requests == 2) {
          if(disconnect) {
            request.get_client().close();
          } else {
            request.set_exception(ServiceRequestException("Denied."));
          }
        } else {
          request.set(std::vector{service.get_account()});
        }
      });
      client->monitor(accounts);
      REQUIRE(accounts->pop() == AccountUpdate::add(service.get_account()));
    } else {
      fixture.on_request<RegisterService>(
        [&] (auto& request, const std::string&, const JsonObject&) {
          ++requests;
          if(requests == 2) {
            if(disconnect) {
              request.get_client().close();
            } else {
              request.set_exception(ServiceRequestException("Denied."));
            }
          } else {
            request.set(service);
          }
        });
      client->add(service.get_name(), service.get_properties());
    }
    fixture.close_server_side(*client);
    flush_pending_routines();
    REQUIRE(requests >= 2);
    if(disconnect) {
      REQUIRE(requests == 3);
    } else if(monitor_accounts) {
      REQUIRE(accounts->is_broken());
      REQUIRE_THROWS_AS(accounts->pop(), ServiceRequestException);
    }
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, marker, true);
    flush_pending_routines();
    REQUIRE((queue->try_pop() == ServiceUpdate::add(marker)));
    auto duplicate = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", duplicate);
    flush_pending_routines();
    REQUIRE((duplicate->try_pop() == ServiceUpdate::add(service)));
    REQUIRE((duplicate->try_pop() == ServiceUpdate::add(marker)));
  }

  TEST_CASE("monitor_services_reconnect_login_failure") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto service = ServiceEntry("quotes", JsonObject(), 1,
      DirectoryEntry::make_account(12, "provider"));
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        request.set(std::vector{service});
      });
    fixture.on_request<MonitorAccountsService>([&] (auto& request) {
      request.set(std::vector{service.get_account()});
    });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
    auto accounts = std::make_shared<Queue<AccountUpdate>>();
    client->monitor(accounts);
    REQUIRE(accounts->pop() == AccountUpdate::add(service.get_account()));
    fixture.on_request<LoginService>(
      [&] (auto& request, const std::string&, const std::string&) {
        request.set_exception(
          ServiceRequestException("Invalid username or password."));
      });
    fixture.close_server_side(*client);
    flush_pending_routines();
    REQUIRE(queue->is_broken());
    REQUIRE_THROWS_AS(queue->pop(), AuthenticationException);
    REQUIRE(accounts->is_broken());
    REQUIRE_THROWS_AS(accounts->pop(), AuthenticationException);
    auto replacement = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", replacement);
    auto other = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("orders", other);
    flush_pending_routines();
    REQUIRE(replacement->is_broken());
    REQUIRE_THROWS_AS(replacement->pop(), AuthenticationException);
    REQUIRE(other->is_broken());
    REQUIRE_THROWS_AS(other->pop(), AuthenticationException);
    auto account_replacement = std::make_shared<Queue<AccountUpdate>>();
    client->monitor(account_replacement);
    flush_pending_routines();
    REQUIRE(account_replacement->is_broken());
    REQUIRE_THROWS_AS(account_replacement->pop(), AuthenticationException);
  }

  TEST_CASE("monitor_closed_queue") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::make_account(12, "provider");
    auto service = ServiceEntry("quotes", JsonObject(), 1, account);
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        server_client = &request.get_client();
        request.set(std::vector{service});
      });
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", queue);
    REQUIRE(queue->pop() == ServiceUpdate::add(service));
    SUBCASE("services") {
      auto closed = std::make_shared<Queue<ServiceUpdate>>();
      closed->close(std::runtime_error("Cancelled."));
      client->monitor("quotes", closed);
    }
    SUBCASE("accounts") {
      fixture.on_request<MonitorAccountsService>([&] (auto& request) {
        request.set(std::vector{account});
      });
      auto accounts = std::make_shared<Queue<AccountUpdate>>();
      client->monitor(accounts);
      REQUIRE(accounts->pop() == AccountUpdate::add(account));
      auto closed = std::make_shared<Queue<AccountUpdate>>();
      closed->close(std::runtime_error("Cancelled."));
      client->monitor(closed);
    }
    auto duplicate = std::make_shared<Queue<ServiceUpdate>>();
    client->monitor("quotes", duplicate);
    flush_pending_routines();
    REQUIRE((duplicate->try_pop() == ServiceUpdate::add(service)));
    send_record_message<ServiceAvailabilityMessage>(
      *server_client, service, false);
    flush_pending_routines();
    REQUIRE((queue->try_pop() == ServiceUpdate::remove(service)));
    REQUIRE((duplicate->try_pop() == ServiceUpdate::remove(service)));
  }

  TEST_CASE("monitor_accounts_recovery_retry") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::make_account(12, "provider");
    auto requests = 0;
    auto server_client = static_cast<
      TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<MonitorAccountsService>([&] (auto& request) {
      server_client = &request.get_client();
      ++requests;
      if(requests == 2) {
        request.set_exception(ServiceRequestException("Unavailable."));
      } else {
        request.set(std::vector{account});
      }
    });
    auto queue = std::make_shared<Queue<AccountUpdate>>();
    client->monitor(queue);
    REQUIRE(queue->pop() == AccountUpdate::add(account));
    fixture.close_server_side(*client);
    flush_pending_routines();
    REQUIRE(queue->is_broken());
    REQUIRE_THROWS_AS(queue->pop(), ServiceRequestException);
    auto replacement = std::make_shared<Queue<AccountUpdate>>();
    client->monitor(replacement);
    flush_pending_routines();
    REQUIRE((replacement->try_pop() == AccountUpdate::add(account)));
    REQUIRE(!replacement->is_broken());
    send_record_message<AccountUpdateMessage>(
      *server_client, AccountUpdate::remove(account));
    flush_pending_routines();
    REQUIRE((replacement->try_pop() == AccountUpdate::remove(account)));
    client->close();
    REQUIRE_THROWS_AS(replacement->pop(), PipeBrokenException);
  }

  TEST_CASE("register_service_during_reconnect") {
    auto remove_before_recovery = false;
    SUBCASE("remove_after_recovery") {}
    SUBCASE("remove_before_recovery") {
      remove_before_recovery = true;
    }
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::make_account(12, "provider");
    auto registered = std::vector<ServiceEntry>();
    auto previous_requests = 0;
    auto current_requests = 0;
    auto next_id = 0;
    fixture.on_request<RegisterService>(
      [&] (auto& request, const std::string& name,
          const JsonObject& properties) {
        if(name == "previous") {
          ++previous_requests;
        } else {
          ++current_requests;
        }
        auto service = ServiceEntry(name, properties, ++next_id, account);
        registered.push_back(service);
        request.set(service);
      });
    fixture.on_request<UnregisterService>([&] (auto& request, int id) {
      std::erase_if(registered, [&] (const auto& service) {
        return service.get_id() == id;
      });
      request.set();
    });
    client->add("previous", JsonObject());
    fixture.on_request<SubscribeAvailabilityService>(
      [&] (auto& request, const std::string&) {
        request.set(std::vector{
          ServiceEntry("quotes", JsonObject(), 100, account)});
      });
    auto ready = Async<void>();
    auto resume = Async<void>();
    auto is_waiting = false;
    client->monitor("quotes", callback<ServiceUpdate>([&] (const auto&) {
      if(!is_waiting) {
        is_waiting = true;
        ready.get_eval().set();
        resume.get();
      }
    }));
    ready.get();
    auto current = [&] {
      auto cleanup = scope::scope_exit([&] {
        resume.get_eval().set();
        flush_pending_routines();
      });
      fixture.close_server_side(*client);
      flush_pending_routines();
      registered.clear();
      next_id = 0;
      auto current = client->add("current", JsonObject());
      if(remove_before_recovery) {
        client->remove(current);
      }
      return current;
    }();
    REQUIRE(previous_requests == 2);
    REQUIRE(current_requests == 1);
    if(!remove_before_recovery) {
      REQUIRE(registered.size() == 2);
      client->remove(current);
    }
    REQUIRE(registered.size() == 1);
    REQUIRE(registered.front().get_name() == "previous");
    client->close();
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

  TEST_CASE("login_from_session_rejected") {
    auto fixture = Fixture();
    auto login_attempted = false;
    auto session_id = std::string("invalid_session");
    auto key = 12345u;
    fixture.on_request<LoginFromSessionService>(
      [&] (auto& request, const std::string& session,
          unsigned int encryption_key) {
        REQUIRE(session == session_id);
        REQUIRE(encryption_key == key);
        login_attempted = true;
        request.set_exception(ServiceRequestException("Session not found."));
      });
    REQUIRE_THROWS_AS(
      fixture.make_session_client(session_id, key), AuthenticationException);
    REQUIRE(login_attempted);
  }

  TEST_CASE("login_from_session_successful") {
    auto fixture = Fixture();
    auto login_attempted = false;
    auto session_id = std::string("encrypted_session");
    auto key = 67890u;
    auto expected_account = DirectoryEntry::make_account(100, "session_user");
    auto expected_session_id = std::string("new_session_12345");
    fixture.on_request<LoginFromSessionService>(
      [&] (auto& request, const std::string& session,
          unsigned int encryption_key) {
        REQUIRE(session == session_id);
        REQUIRE(encryption_key == key);
        login_attempted = true;
        request.set(LoginServiceResult(expected_account, expected_session_id));
      });
    auto client = fixture.make_session_client(session_id, key);
    REQUIRE(login_attempted);
    REQUIRE(client->get_account() == expected_account);
    REQUIRE(client->get_session_id() == expected_session_id);
  }

  TEST_CASE("login_from_session_can_use_services") {
    auto fixture = Fixture();
    auto expected_account = DirectoryEntry::make_account(100, "session_user");
    fixture.on_request<LoginFromSessionService>(
      [&] (auto& request, const std::string& session,
          unsigned int encryption_key) {
        request.set(LoginServiceResult(expected_account, "new_session"));
      });
    auto client = fixture.make_session_client("encrypted_session", 12345u);
    auto locate_attempted = false;
    fixture.on_request<LocateService>(
      [&] (auto& request, const std::string& name) {
        REQUIRE(name == "test_service");
        locate_attempted = true;
        request.set(std::vector<ServiceEntry>());
      });
    auto result = client->locate("test_service");
    REQUIRE(locate_attempted);
    REQUIRE(result.empty());
  }
}
