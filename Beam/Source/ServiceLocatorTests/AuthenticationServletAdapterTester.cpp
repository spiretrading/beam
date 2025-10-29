#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ProtocolServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"
#include "Beam/ServicesTests/TestServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Fixture {
    using TestServiceLocatorClient =
      ProtocolServiceLocatorClient<TestServiceProtocolClientBuilder>;
    using MetaServlet = MetaAuthenticationServletAdapter<
      MetaTestServlet, std::unique_ptr<TestServiceLocatorClient>>;
    using ServiceProtocolServletContainer =
      TestServiceProtocolServletContainer<MetaServlet>;
    using Servlet = AuthenticationServletAdapter<
      ServiceProtocolServletContainer,
      TestServlet<ServiceProtocolServletContainer>,
      std::unique_ptr<TestServiceLocatorClient>>;
    using ServiceLocatorContainer = TestServiceProtocolServletContainer<
      MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>>;
    LocalServiceLocatorDataStore m_data_store;
    optional<ServiceLocatorContainer> m_service_locator_container;
    optional<TestServiceLocatorClient> m_service_locator_client;
    optional<ServiceProtocolServletContainer> m_container;
    optional<TestServiceProtocolClient> m_client_protocol;

    Fixture() {
      auto service_locator_server_connection =
        std::make_shared<LocalServerConnection>();
      m_service_locator_container.emplace(
        &m_data_store, service_locator_server_connection,
        factory<std::unique_ptr<TriggerTimer>>());
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", *service_locator_server_connection);
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      auto time = time_from_string("2024-01-01 12:00:00");
      m_data_store.make_account(
        "test", "1234", DirectoryEntry::STAR_DIRECTORY, time);
      m_data_store.make_account(
        "test2", "1234", DirectoryEntry::STAR_DIRECTORY, time);
      m_service_locator_client.emplace("test2", "1234", builder);
      auto service_client =
        std::make_unique<TestServiceLocatorClient>("test", "1234", builder);
      auto server_connection = std::make_shared<LocalServerConnection>();
      m_container.emplace(init(std::move(service_client), init()),
        server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_protocol.emplace(std::make_unique<LocalClientChannel>(
        "test", *server_connection), init());
      register_test_services(out(m_client_protocol->get_slots()));
    }
  };
}

TEST_SUITE("AuthenticationServletAdapter") {
  TEST_CASE_FIXTURE(Fixture, "service_without_authentication") {
    REQUIRE_THROWS_AS(m_client_protocol->send_request<VoidService>(123),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "service_with_authentication") {
    auto authenticator = SessionAuthenticator(Ref(*m_service_locator_client));
    authenticator(*m_client_protocol);
    m_client_protocol->send_request<VoidService>(123);
  }
}
