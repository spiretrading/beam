#include "Beam/ServiceLocatorTests/AuthenticationServletAdapterTester.hpp"
#include <boost/functional/factory.hpp>
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"

using namespace Beam;
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
  auto serviceLocatorServerConnection =
    std::make_unique<TestServerConnection>();
  auto serviceLocatorServerConnectionHandle =
    serviceLocatorServerConnection.get();
  TestServiceProtocolClientBuilder builder{
    [=] {
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>("test",
        Ref(*serviceLocatorServerConnectionHandle));
    }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>()};
  m_serviceLocatorClient.emplace(builder);
  auto serviceClient = std::make_unique<TestServiceLocatorClient>(builder);
  m_serviceLocatorContainer.emplace(m_dataStore,
    std::move(serviceLocatorServerConnection),
    factory<std::unique_ptr<TriggerTimer>>());
  m_serviceLocatorContainer->Open();
  m_dataStore->MakeAccount("test", "1234", DirectoryEntry::GetStarDirectory(),
    second_clock::universal_time());
  m_dataStore->MakeAccount("test2", "1234", DirectoryEntry::GetStarDirectory(),
    second_clock::universal_time());
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

void AuthenticationServletAdapterTester::tearDown() {
  m_container.reset();
  m_serviceLocatorClient.reset();
  m_clientProtocol.reset();
  m_serviceLocatorContainer.reset();
  m_dataStore.reset();
}

void AuthenticationServletAdapterTester::TestServiceWithoutAuthentication() {
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<VoidService>(123),
    ServiceRequestException);
}

void AuthenticationServletAdapterTester::TestServiceWithAuthentication() {
  SessionAuthenticator<TestServiceLocatorClient> authenticator{
    Ref(*m_serviceLocatorClient)};
  authenticator(*m_clientProtocol);
  m_clientProtocol->SendRequest<VoidService>(123);
}
