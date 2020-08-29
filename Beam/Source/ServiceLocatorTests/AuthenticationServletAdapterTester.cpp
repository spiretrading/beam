#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"
#include "Beam/ServicesTests/TestServlet.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Fixture {
    using TestServiceLocatorClient = ServiceLocatorClient<
      TestServiceProtocolClientBuilder>;
    using MetaServlet = MetaAuthenticationServletAdapter<MetaTestServlet,
      std::unique_ptr<TestServiceLocatorClient>>;
    using ServiceProtocolServletContainer =
      TestServiceProtocolServletContainer<MetaServlet>;
    using Servlet = AuthenticationServletAdapter<
      ServiceProtocolServletContainer,
      TestServlet<ServiceProtocolServletContainer>,
      std::unique_ptr<TestServiceLocatorClient>>;
    using ServiceLocatorContainer = TestServiceProtocolServletContainer<
      MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>>;

    LocalServiceLocatorDataStore m_dataStore;
    optional<ServiceLocatorContainer> m_serviceLocatorContainer;
    optional<TestServiceLocatorClient> m_serviceLocatorClient;
    optional<ServiceProtocolServletContainer> m_container;
    optional<TestServiceProtocolClient> m_clientProtocol;

    Fixture() {
      auto serviceLocatorServerConnection =
        std::make_shared<TestServerConnection>();
      m_serviceLocatorContainer.emplace(&m_dataStore,
        serviceLocatorServerConnection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_serviceLocatorContainer->Open();
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", *serviceLocatorServerConnection);
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_serviceLocatorClient.emplace(builder);
      auto serviceClient = std::make_unique<TestServiceLocatorClient>(builder);
      m_dataStore.MakeAccount("test", "1234",
        DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
      m_dataStore.MakeAccount("test2", "1234",
        DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
      serviceClient->SetCredentials("test", "1234");
      serviceClient->Open();
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_container.emplace(Initialize(std::move(serviceClient), Initialize()),
        serverConnection, factory<std::unique_ptr<TriggerTimer>>());
      m_container->Open();
      m_clientProtocol.emplace(Initialize("test", *serverConnection),
        Initialize());
      RegisterTestServices(Store(m_clientProtocol->GetSlots()));
      m_clientProtocol->Open();
      m_serviceLocatorClient->SetCredentials("test2", "1234");
      m_serviceLocatorClient->Open();
    }
  };
}

TEST_SUITE("AuthenticationServletAdapter") {
  TEST_CASE_FIXTURE(Fixture, "service_without_authentication") {
    REQUIRE_THROWS_AS(m_clientProtocol->SendRequest<VoidService>(123),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "service_with_authentication") {
    auto authenticator = SessionAuthenticator(Ref(*m_serviceLocatorClient));
    authenticator(*m_clientProtocol);
    m_clientProtocol->SendRequest<VoidService>(123);
  }
}
