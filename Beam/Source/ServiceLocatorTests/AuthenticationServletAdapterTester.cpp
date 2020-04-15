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
    boost::optional<ServiceProtocolServletContainer> m_container;
    boost::optional<Services::Tests::TestServiceProtocolClient>
      m_clientProtocol;
    boost::optional<ServiceLocatorContainer> m_serviceLocatorContainer;
    boost::optional<TestServiceLocatorClient> m_serviceLocatorClient;

    Fixture() {
      auto serviceLocatorServerConnection =
        std::make_unique<TestServerConnection>();
      auto serviceLocatorServerConnectionHandle =
        serviceLocatorServerConnection.get();
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", Ref(*serviceLocatorServerConnectionHandle));
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_serviceLocatorClient.emplace(builder);
      auto serviceClient = std::make_unique<TestServiceLocatorClient>(builder);
      m_serviceLocatorContainer.emplace(&m_dataStore,
        std::move(serviceLocatorServerConnection),
        factory<std::unique_ptr<TriggerTimer>>());
      m_serviceLocatorContainer->Open();
      m_dataStore.MakeAccount("test", "1234",
        DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
      m_dataStore.MakeAccount("test2", "1234",
        DirectoryEntry::GetStarDirectory(), second_clock::universal_time());
      auto serverConnection = std::make_unique<TestServerConnection>();
      m_clientProtocol.emplace(Initialize("test", Ref(*serverConnection)),
        Initialize());
      RegisterTestServices(Store(m_clientProtocol->GetSlots()));
      serviceClient->SetCredentials("test", "1234");
      serviceClient->Open();
      m_container.emplace(Initialize(std::move(serviceClient), Initialize()),
        std::move(serverConnection), factory<std::unique_ptr<TriggerTimer>>());
      m_container->Open();
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
