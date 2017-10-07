#include "Beam/RegistryServiceTests/RegistryServletTester.hpp"
#include <boost/functional/factory.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::RegistryService;
using namespace Beam::RegistryService::Tests;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace std;

void RegistryServletTester::setUp() {
  m_serviceLocatorEnvironment.emplace();
  m_serviceLocatorEnvironment->Open();
  m_dataStore = std::make_shared<LocalRegistryDataStore>();
  auto serverConnection = std::make_unique<TestServerConnection>();
  m_clientProtocol.emplace(Initialize("test", Ref(*serverConnection)),
    Initialize());
  RegisterServiceLocatorServices(Store(m_clientProtocol->GetSlots()));
  RegisterServiceLocatorMessages(Store(m_clientProtocol->GetSlots()));
  RegisterRegistryServices(Store(m_clientProtocol->GetSlots()));
  auto registryServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  registryServiceLocatorClient->SetCredentials("root", "");
  registryServiceLocatorClient->Open();
  m_container.emplace(Initialize(std::move(registryServiceLocatorClient),
    Initialize(m_dataStore)), std::move(serverConnection),
    factory<std::unique_ptr<TriggerTimer>>());
  m_container->Open();
}

void RegistryServletTester::tearDown() {
  m_clientProtocol.reset();
  m_container.reset();
  m_dataStore.reset();
}

void RegistryServletTester::TestMakeDirectory() {
  auto serviceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  serviceLocatorClient->SetCredentials("root", "");
  serviceLocatorClient->Open();
  OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
    Ref(*serviceLocatorClient)), *m_clientProtocol);
  auto directoryName = string{"directory"};
  RegistryEntry directory = m_clientProtocol->SendRequest<MakeDirectoryService>(
    directoryName, RegistryEntry::GetRoot());
}

void RegistryServletTester::TestMakeValue() {
  auto serviceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  serviceLocatorClient->SetCredentials("root", "");
  serviceLocatorClient->Open();
  OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
    Ref(*serviceLocatorClient)), *m_clientProtocol);
  auto key = string{"key"};
  auto value = BufferFromString<SharedBuffer>("value");
  auto entry = m_clientProtocol->SendRequest<MakeValueService>(key, value,
    RegistryEntry::GetRoot());
}

void RegistryServletTester::TestLoadPath() {
  auto serviceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  serviceLocatorClient->SetCredentials("root", "");
  serviceLocatorClient->Open();
  OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
    Ref(*serviceLocatorClient)), *m_clientProtocol);
  RegistryEntry directory(RegistryEntry::Type::DIRECTORY, 0, "directory", 0);
  directory = m_dataStore->Copy(directory, RegistryEntry::GetRoot());
  RegistryEntry pathLoaded = m_clientProtocol->SendRequest<LoadPathService>(
    RegistryEntry::GetRoot(), "directory");
  CPPUNIT_ASSERT(pathLoaded == directory);
}

void RegistryServletTester::TestLoadValue() {
  auto serviceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  serviceLocatorClient->SetCredentials("root", "");
  serviceLocatorClient->Open();
  OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
    Ref(*serviceLocatorClient)), *m_clientProtocol);
  RegistryEntry valueEntry(RegistryEntry::Type::VALUE, 0, "value", 0);
  valueEntry = m_dataStore->Copy(valueEntry, RegistryEntry::GetRoot());
  auto value = BufferFromString<SharedBuffer>("value");
  valueEntry = m_dataStore->Store(valueEntry, value);
  auto loadedValue = m_clientProtocol->SendRequest<LoadValueService>(
    valueEntry);
  CPPUNIT_ASSERT(loadedValue == value);
}
