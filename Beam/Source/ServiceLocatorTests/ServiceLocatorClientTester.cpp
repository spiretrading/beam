#include <future>
#include <doctest/doctest.h>
#include "Beam/ServiceLocatorTests/TestServiceLocatorClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

TEST_SUITE("ServiceLocatorClient") {
  TEST_CASE("load_or_create_directory_existing") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto parent = DirectoryEntry::make_directory(100, "parent");
    auto directory_name = std::string("existing_directory");
    auto expected_directory =
      DirectoryEntry::make_directory(200, directory_name);
    auto future = std::async(std::launch::async, [&] {
      return load_or_create_directory(client, directory_name, parent);
    });
    auto operation = operations->pop();
    auto& load_operation =
      std::get<TestServiceLocatorClient::LoadDirectoryEntryByPathOperation>(
        *operation);
    REQUIRE(load_operation.m_root == parent);
    REQUIRE(load_operation.m_path == directory_name);
    load_operation.m_result.set(expected_directory);
    auto result = future.get();
    REQUIRE(result == expected_directory);
  }

  TEST_CASE("load_or_create_directory_create_new") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto parent = DirectoryEntry::make_directory(100, "parent");
    auto directory_name = std::string("new_directory");
    auto expected_directory =
      DirectoryEntry::make_directory(300, directory_name);
    auto future = std::async(std::launch::async, [&] {
      return load_or_create_directory(client, directory_name, parent);
    });
    auto load_operation = operations->pop();
    auto& load_op = std::get<
      TestServiceLocatorClient::LoadDirectoryEntryByPathOperation>(
        *load_operation);
    REQUIRE(load_op.m_root == parent);
    REQUIRE(load_op.m_path == directory_name);
    load_op.m_result.set(
      std::make_exception_ptr(ServiceRequestException("Directory not found.")));
    auto make_operation = operations->pop();
    auto& make_op = std::get<TestServiceLocatorClient::MakeDirectoryOperation>(
      *make_operation);
    REQUIRE(make_op.m_name == directory_name);
    REQUIRE(make_op.m_parent == parent);
    make_op.m_result.set(expected_directory);
    auto result = future.get();
    REQUIRE(result == expected_directory);
  }

  TEST_CASE("locate_service_addresses_no_services") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto service_name = std::string("nonexistent_service");
    auto future = std::async(std::launch::async, [&] {
      return locate_service_addresses(client, service_name);
    });
    auto operation = operations->pop();
    auto& locate_op =
      std::get<TestServiceLocatorClient::LocateOperation>(*operation);
    REQUIRE(locate_op.m_name == service_name);
    locate_op.m_result.set(
      std::make_exception_ptr(ServiceRequestException("Service not found.")));
    REQUIRE_THROWS_AS(future.get(), ConnectException);
  }

  TEST_CASE("locate_service_addresses_empty_list") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto service_name = std::string("empty_service");
    auto future = std::async(std::launch::async, [&] {
      return locate_service_addresses(client, service_name);
    });
    auto operation = operations->pop();
    auto& locate_op =
      std::get<TestServiceLocatorClient::LocateOperation>(*operation);
    REQUIRE(locate_op.m_name == service_name);
    locate_op.m_result.set(std::vector<ServiceEntry>());
    REQUIRE_THROWS_AS(future.get(), ConnectException);
  }

  TEST_CASE("locate_service_addresses_single_service") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto service_name = std::string("test_service");
    auto service_account = DirectoryEntry::make_account(100, "service_owner");
    auto properties = JsonObject();
    properties.set("addresses", "[192.168.1.1:8080, 192.168.1.2:8081]");
    auto service = ServiceEntry(service_name, properties, 1, service_account);
    auto future = std::async(std::launch::async, [&] {
      return locate_service_addresses(client, service_name);
    });
    auto operation = operations->pop();
    auto& locate_op =
      std::get<TestServiceLocatorClient::LocateOperation>(*operation);
    REQUIRE(locate_op.m_name == service_name);
    auto services = std::vector<ServiceEntry>();
    services.push_back(service);
    locate_op.m_result.set(services);
    auto result = future.get();
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].get_host() == "192.168.1.1");
    REQUIRE(result[0].get_port() == 8080);
    REQUIRE(result[1].get_host() == "192.168.1.2");
    REQUIRE(result[1].get_port() == 8081);
  }

  TEST_CASE("locate_service_addresses_multiple_services") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto service_name = std::string("distributed_service");
    auto service_account = DirectoryEntry::make_account(100, "service_owner");
    auto properties_one = JsonObject();
    properties_one.set("addresses", "[10.0.0.1:9000]");
    auto service_one =
      ServiceEntry(service_name, properties_one, 1, service_account);
    auto properties_two = JsonObject();
    properties_two.set("addresses", "[10.0.0.2:9000]");
    auto service_two =
      ServiceEntry(service_name, properties_two, 2, service_account);
    auto properties_three = JsonObject();
    properties_three.set("addresses", "[10.0.0.3:9000]");
    auto service_three =
      ServiceEntry(service_name, properties_three, 3, service_account);
    auto future = std::async(std::launch::async, [&] {
      return locate_service_addresses(client, service_name);
    });
    auto operation = operations->pop();
    auto& locate_op =
      std::get<TestServiceLocatorClient::LocateOperation>(*operation);
    REQUIRE(locate_op.m_name == service_name);
    auto services = std::vector<ServiceEntry>();
    services.push_back(service_one);
    services.push_back(service_two);
    services.push_back(service_three);
    locate_op.m_result.set(services);
    auto result = future.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].get_port() == 9000);
  }

  TEST_CASE("locate_service_addresses_with_predicate_all_match") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto service_name = std::string("filtered_service");
    auto service_account = DirectoryEntry::make_account(100, "service_owner");
    auto properties_one = JsonObject();
    properties_one.set("version", 2);
    properties_one.set("addresses", "[172.16.0.1:5000]");
    auto service_one =
      ServiceEntry(service_name, properties_one, 1, service_account);
    auto properties_two = JsonObject();
    properties_two.set("version", 2);
    properties_two.set("addresses", "[172.16.0.2:5000]");
    auto service_two =
      ServiceEntry(service_name, properties_two, 2, service_account);
    auto future = std::async(std::launch::async, [&] {
      return locate_service_addresses(client, service_name,
        [] (const auto& entry) {
          auto version_value = entry.get_properties().get("version");
          return version_value.has_value() && get<double>(*version_value) == 2;
        });
    });
    auto operation = operations->pop();
    auto& locate_op =
      std::get<TestServiceLocatorClient::LocateOperation>(*operation);
    REQUIRE(locate_op.m_name == service_name);
    auto services = std::vector<ServiceEntry>();
    services.push_back(service_one);
    services.push_back(service_two);
    locate_op.m_result.set(services);
    auto result = future.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].get_port() == 5000);
  }

  TEST_CASE("locate_service_addresses_with_predicate_none_match") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto service_name = std::string("filtered_service");
    auto service_account = DirectoryEntry::make_account(100, "service_owner");
    auto properties_one = JsonObject();
    properties_one.set("version", 1);
    properties_one.set("addresses", "[172.16.0.1:5000]");
    auto service_one =
      ServiceEntry(service_name, properties_one, 1, service_account);
    auto properties_two = JsonObject();
    properties_two.set("version", 1);
    properties_two.set("addresses", "[172.16.0.2:5000]");
    auto service_two =
      ServiceEntry(service_name, properties_two, 2, service_account);
    auto future = std::async(std::launch::async, [&] {
      return locate_service_addresses(client, service_name,
        [] (const auto& entry) {
          auto version_value = entry.get_properties().get("version");
          return version_value.has_value() && get<double>(*version_value) == 2;
        });
    });
    auto operation = operations->pop();
    auto& locate_op =
      std::get<TestServiceLocatorClient::LocateOperation>(*operation);
    REQUIRE(locate_op.m_name == service_name);
    auto services = std::vector<ServiceEntry>();
    services.push_back(service_one);
    services.push_back(service_two);
    locate_op.m_result.set(services);
    REQUIRE_THROWS_AS(future.get(), ConnectException);
  }

  TEST_CASE("locate_service_addresses_with_predicate_partial_match") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(1, "test_user");
    auto session_id = std::string("test_session");
    auto client = TestServiceLocatorClient(account, session_id, operations);
    auto service_name = std::string("mixed_service");
    auto service_account = DirectoryEntry::make_account(100, "service_owner");
    auto properties_one = JsonObject();
    properties_one.set("region", "east");
    properties_one.set("addresses", "[192.168.10.1:7000]");
    auto service_one =
      ServiceEntry(service_name, properties_one, 1, service_account);
    auto properties_two = JsonObject();
    properties_two.set("region", "west");
    properties_two.set("addresses", "[192.168.10.2:7000]");
    auto service_two =
      ServiceEntry(service_name, properties_two, 2, service_account);
    auto properties_three = JsonObject();
    properties_three.set("region", "east");
    properties_three.set("addresses", "[192.168.10.3:7000]");
    auto service_three =
      ServiceEntry(service_name, properties_three, 3, service_account);
    auto future = std::async(std::launch::async, [&] {
      return locate_service_addresses(client, service_name,
        [] (const auto& entry) {
          auto region_value = entry.get_properties().get("region");
          return region_value.has_value() &&
            get<std::string>(*region_value) == "east";
        });
    });
    auto operation = operations->pop();
    auto& locate_op =
      std::get<TestServiceLocatorClient::LocateOperation>(*operation);
    REQUIRE(locate_op.m_name == service_name);
    auto services = std::vector<ServiceEntry>();
    services.push_back(service_one);
    services.push_back(service_two);
    services.push_back(service_three);
    locate_op.m_result.set(services);
    auto result = future.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].get_port() == 7000);
  }
}
