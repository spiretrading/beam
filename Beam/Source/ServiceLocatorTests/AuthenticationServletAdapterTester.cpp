#include "Beam/ServiceLocatorTests/AuthenticationServletAdapterTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

void AuthenticationServletAdapterTester::setUp() {
  m_dataStore = std::make_shared<LocalServiceLocatorDataStore>();
  auto serviceLocatorServerConnection = std::make_unique<ServerConnection>();
  auto serviceLocatorServerConnectionHandle =
    serviceLocatorServerConnection.get();
  ServiceProtocolClientBuilder builder(
    [=] {
      return std::make_unique<ServiceProtocolClientBuilder::Channel>(("test"),
        Ref(*serviceLocatorServerConnectionHandle));
    },
    [] {
      return std::make_unique<ServiceProtocolClientBuilder::Timer>();
    });
  m_serviceLocatorClient.Initialize(builder);
  auto serviceClient = std::make_unique<TestServiceLocatorClient>(builder);
  m_serviceLocatorContainer.Initialize(m_dataStore,
    std::move(serviceLocatorServerConnection),
    factory<std::shared_ptr<TriggerTimer>>());
  m_serviceLocatorContainer->Open();
  m_dataStore->MakeAccount("test", "1234", DirectoryEntry::GetStarDirectory(),
    second_clock::universal_time());
  m_dataStore->MakeAccount("test2", "1234", DirectoryEntry::GetStarDirectory(),
    second_clock::universal_time());
  auto serverConnection = std::make_unique<ServerConnection>();
  m_clientProtocol.Initialize(Initialize(string("test"),
    Ref(*serverConnection)), Initialize());
  RegisterTestServices(Store(m_clientProtocol->GetSlots()));
  serviceClient->SetCredentials("test", "1234");
  serviceClient->Open();
  m_container.Initialize(Initialize(std::move(serviceClient), Initialize()),
    std::move(serverConnection), factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
  m_serviceLocatorClient->SetCredentials("test2", "1234");
  m_serviceLocatorClient->Open();
}

void AuthenticationServletAdapterTester::tearDown() {
  m_container.Reset();
  m_serviceLocatorClient.Reset();
  m_clientProtocol.Reset();
  m_serviceLocatorContainer.Reset();
}

void AuthenticationServletAdapterTester::TestServiceWithoutAuthentication() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<VoidService>(123),
    ServiceRequestException);
}

void AuthenticationServletAdapterTester::TestServiceWithAuthentication() {
  SessionAuthenticator<TestServiceLocatorClient> authenticator(
    Ref(*m_serviceLocatorClient));
  authenticator(*m_clientProtocol);
  m_clientProtocol->SendRequest<VoidService>(123);
}
