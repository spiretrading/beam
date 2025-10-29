#include <ranges>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/ServiceLocatorSession.hpp"

using namespace Beam;

TEST_SUITE("ServiceLocatorSession") {
  TEST_CASE("default_construction") {
    auto session = ServiceLocatorSession();
    REQUIRE(session.get_session_id().empty());
    REQUIRE(!session.is_logged_in());
    REQUIRE(session.get_service_subscriptions().empty());
    REQUIRE(session.get_registered_services().empty());
    REQUIRE(session.get_monitors().empty());
  }

  TEST_CASE("move_construction") {
    auto session1 = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "test_account");
    session1.set_session_id(account, "session123");
    session1.subscribe_service("test_service");
    auto entry = DirectoryEntry::make_directory(2, "test_dir");
    session1.monitor(entry);
    auto session2 = std::move(session1);
    REQUIRE(session2.get_session_id() == "session123");
    REQUIRE(session2.is_logged_in());
    REQUIRE(session2.get_account() == account);
    auto subscriptions = session2.get_service_subscriptions();
    REQUIRE(subscriptions.size() == 1);
    REQUIRE(subscriptions[0] == "test_service");
    auto monitors = session2.get_monitors();
    REQUIRE(monitors.size() == 1);
    REQUIRE(monitors[0] == entry);
  }

  TEST_CASE("set_and_get_session_id") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "test_account");
    session.set_session_id(account, "abc123");
    REQUIRE(session.get_session_id() == "abc123");
    REQUIRE(session.is_logged_in());
    REQUIRE(session.get_account() == account);
  }

  TEST_CASE("try_login_initial") {
    auto session = ServiceLocatorSession();
    auto result = session.try_login();
    REQUIRE(result);
  }

  TEST_CASE("try_login_while_logging_in") {
    auto session = ServiceLocatorSession();
    session.try_login();
    auto result = session.try_login();
    REQUIRE(!result);
  }

  TEST_CASE("try_login_while_logged_in") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "test");
    session.set_session_id(account, "session");
    auto result = session.try_login();
    REQUIRE(!result);
  }

  TEST_CASE("reset_login") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "test");
    session.set_session_id(account, "session123");
    REQUIRE(session.is_logged_in());
    session.reset_login();
    REQUIRE(!session.is_logged_in());
    REQUIRE(session.get_session_id().empty());
  }

  TEST_CASE("reset_login_allows_new_login") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "test");
    session.set_session_id(account, "session");
    session.reset_login();
    auto result = session.try_login();
    REQUIRE(result);
  }

  TEST_CASE("subscribe_single_service") {
    auto session = ServiceLocatorSession();
    session.subscribe_service("market_data");
    auto subscriptions = session.get_service_subscriptions();
    REQUIRE(subscriptions.size() == 1);
    REQUIRE(subscriptions[0] == "market_data");
  }

  TEST_CASE("subscribe_multiple_services") {
    auto session = ServiceLocatorSession();
    session.subscribe_service("market_data");
    session.subscribe_service("order_execution");
    session.subscribe_service("risk_management");
    auto subscriptions = session.get_service_subscriptions();
    REQUIRE(subscriptions.size() == 3);
    REQUIRE(std::ranges::contains(subscriptions, "market_data"));
    REQUIRE(std::ranges::contains(subscriptions, "order_execution"));
    REQUIRE(std::ranges::contains(subscriptions, "risk_management"));
  }

  TEST_CASE("unsubscribe_service") {
    auto session = ServiceLocatorSession();
    session.subscribe_service("market_data");
    session.subscribe_service("order_execution");
    session.unsubscribe_service("market_data");
    auto subscriptions = session.get_service_subscriptions();
    REQUIRE(subscriptions.size() == 1);
    REQUIRE(subscriptions[0] == "order_execution");
  }

  TEST_CASE("unsubscribe_nonexistent_service") {
    auto session = ServiceLocatorSession();
    session.subscribe_service("market_data");
    session.unsubscribe_service("nonexistent");
    auto subscriptions = session.get_service_subscriptions();
    REQUIRE(subscriptions.size() == 1);
    REQUIRE(subscriptions[0] == "market_data");
  }

  TEST_CASE("register_single_service") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "service_account");
    auto service = ServiceEntry("test_service", JsonObject(), 100, account);
    session.register_service(service);
    auto services = session.get_registered_services();
    REQUIRE(services.size() == 1);
    REQUIRE(services[0].get_id() == 100);
    REQUIRE(services[0].get_name() == "test_service");
  }

  TEST_CASE("register_multiple_services") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "service_account");
    auto service1 = ServiceEntry("service1", JsonObject(), 100, account);
    auto service2 = ServiceEntry("service2", JsonObject(), 101, account);
    auto service3 = ServiceEntry("service3", JsonObject(), 102, account);
    session.register_service(service1);
    session.register_service(service2);
    session.register_service(service3);
    auto services = session.get_registered_services();
    REQUIRE(services.size() == 3);
    REQUIRE(std::ranges::any_of(services, [] (const auto& service) {
      return service.get_id() == 100 && service.get_name() == "service1";
    }));
    REQUIRE(std::ranges::any_of(services, [] (const auto& service) {
      return service.get_id() == 101 && service.get_name() == "service2";
    }));
    REQUIRE(std::ranges::any_of(services, [] (const auto& service) {
      return service.get_id() == 102 && service.get_name() == "service3";
    }));
  }

  TEST_CASE("unregister_service") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "service_account");
    auto service1 = ServiceEntry("service1", JsonObject(), 100, account);
    auto service2 = ServiceEntry("service2", JsonObject(), 101, account);
    session.register_service(service1);
    session.register_service(service2);
    session.unregister_service(100);
    auto services = session.get_registered_services();
    REQUIRE(services.size() == 1);
    REQUIRE(services[0].get_id() == 101);
    REQUIRE(services[0].get_name() == "service2");
  }

  TEST_CASE("unregister_nonexistent_service") {
    auto session = ServiceLocatorSession();
    auto account = DirectoryEntry::make_account(1, "service_account");
    auto service = ServiceEntry("service", JsonObject(), 100, account);
    session.register_service(service);
    session.unregister_service(999);
    auto services = session.get_registered_services();
    REQUIRE(services.size() == 1);
    REQUIRE(services[0].get_id() == 100);
    REQUIRE(services[0].get_name() == "service");
  }

  TEST_CASE("monitor_single_entry") {
    auto session = ServiceLocatorSession();
    auto entry = DirectoryEntry::make_directory(1, "directory");
    session.monitor(entry);
    auto monitors = session.get_monitors();
    REQUIRE(monitors.size() == 1);
    REQUIRE(monitors[0] == entry);
  }

  TEST_CASE("monitor_multiple_entries") {
    auto session = ServiceLocatorSession();
    auto entry1 = DirectoryEntry::make_directory(1, "dir1");
    auto entry2 = DirectoryEntry::make_account(2, "account1");
    auto entry3 = DirectoryEntry::make_directory(3, "dir2");
    session.monitor(entry1);
    session.monitor(entry2);
    session.monitor(entry3);
    auto monitors = session.get_monitors();
    REQUIRE(monitors.size() == 3);
    REQUIRE(std::ranges::contains(monitors, entry1));
    REQUIRE(std::ranges::contains(monitors, entry2));
    REQUIRE(std::ranges::contains(monitors, entry3));
  }

  TEST_CASE("unmonitor_entry") {
    auto session = ServiceLocatorSession();
    auto entry1 = DirectoryEntry::make_directory(1, "dir1");
    auto entry2 = DirectoryEntry::make_account(2, "account1");
    session.monitor(entry1);
    session.monitor(entry2);
    session.unmonitor(entry1);
    auto monitors = session.get_monitors();
    REQUIRE(monitors.size() == 1);
    REQUIRE(monitors[0] == entry2);
  }

  TEST_CASE("unmonitor_nonexistent_entry") {
    auto session = ServiceLocatorSession();
    auto entry1 = DirectoryEntry::make_directory(1, "dir1");
    auto entry2 = DirectoryEntry::make_directory(2, "dir2");
    session.monitor(entry1);
    session.unmonitor(entry2);
    auto monitors = session.get_monitors();
    REQUIRE(monitors.size() == 1);
    REQUIRE(monitors[0] == entry1);
  }

  TEST_CASE("complex_workflow") {
    auto session = ServiceLocatorSession();
    REQUIRE(session.try_login());
    auto account = DirectoryEntry::make_account(1, "user");
    session.set_session_id(account, "session_xyz");
    session.subscribe_service("service1");
    session.subscribe_service("service2");
    auto service_account = DirectoryEntry::make_account(2, "service_provider");
    auto service = ServiceEntry(
      "my_service", JsonObject(), 100, service_account);
    session.register_service(service);
    auto directory = DirectoryEntry::make_directory(3, "watched");
    session.monitor(directory);
    REQUIRE(session.is_logged_in());
    REQUIRE(session.get_session_id() == "session_xyz");
    REQUIRE(session.get_account() == account);
    auto subscriptions = session.get_service_subscriptions();
    REQUIRE(subscriptions.size() == 2);
    REQUIRE(std::ranges::contains(subscriptions, "service1"));
    REQUIRE(std::ranges::contains(subscriptions, "service2"));
    auto services = session.get_registered_services();
    REQUIRE(services.size() == 1);
    REQUIRE(services[0].get_id() == 100);
    REQUIRE(services[0].get_name() == "my_service");
    auto monitors = session.get_monitors();
    REQUIRE(monitors.size() == 1);
    REQUIRE(monitors[0] == directory);
    session.unsubscribe_service("service1");
    session.unregister_service(100);
    session.unmonitor(directory);
    subscriptions = session.get_service_subscriptions();
    REQUIRE(subscriptions.size() == 1);
    REQUIRE(subscriptions[0] == "service2");
    REQUIRE(session.get_registered_services().empty());
    REQUIRE(session.get_monitors().empty());
    session.reset_login();
    REQUIRE(!session.is_logged_in());
    REQUIRE(session.get_session_id().empty());
  }
}
