#include <doctest/doctest.h>
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"
#include "Beam/ServiceLocatorTests/TestServiceLocatorClient.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ServiceMonitoring") {
  TEST_CASE("registrations") {
    auto environment = ServiceLocatorTestEnvironment();
    auto provider = environment.make_client();
    auto subscriber = environment.make_client();
    auto properties = JsonObject();
    properties.set("scope", "TSX");
    auto first = provider.add("quotes", properties);
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    subscriber.monitor("quotes", queue);
    auto update = queue->pop();
    REQUIRE(update == ServiceUpdate::add(first));
    REQUIRE(update.m_service.get_properties() == properties);
    provider.add("orders", JsonObject());
    auto second = provider.add("quotes", JsonObject());
    REQUIRE(queue->pop() == ServiceUpdate::add(second));
    provider.remove(first);
    REQUIRE(queue->pop() == ServiceUpdate::remove(first));
    provider.close();
    REQUIRE(queue->pop() == ServiceUpdate::remove(second));
    subscriber.close();
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
  }

  TEST_CASE("test_client") {
    auto operations = std::make_shared<TestServiceLocatorClient::Queue>();
    auto account = DirectoryEntry::make_account(12, "provider");
    auto client = ServiceLocatorClient(
      std::in_place_type<TestServiceLocatorClient>, account, "", operations);
    auto queue = std::make_shared<Queue<ServiceUpdate>>();
    client.monitor("quotes", queue);
    auto operation = operations->pop();
    auto& monitor =
      std::get<TestServiceLocatorClient::MonitorServicesOperation>(*operation);
    REQUIRE(monitor.m_name == "quotes");
    auto update = ServiceUpdate::add(
      ServiceEntry("quotes", JsonObject(), 1, account));
    monitor.m_queue.push(update);
    REQUIRE(queue->pop() == update);
    client.close();
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
  }
}
