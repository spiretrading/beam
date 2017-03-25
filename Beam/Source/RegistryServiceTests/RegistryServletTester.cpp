#include "Beam/RegistryServiceTests/RegistryServletTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Beam/SignalHandling/NullSlot.hpp"
#include "Beam/ServiceLocator/Authenticator.hpp"
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::RegistryService;
using namespace Beam::RegistryService::Tests;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

void RegistryServletTester::setUp() {
  m_serviceLocatorEnvironment.Initialize();
  m_serviceLocatorEnvironment->Open();
  m_dataStore = std::make_shared<LocalRegistryDataStore>();
  m_serverConnection.Initialize();
  m_clientProtocol.Initialize(Initialize(string("test"),
    Ref(*m_serverConnection)), Initialize());
  RegisterServiceLocatorServices(Store(m_clientProtocol->GetSlots()));
  RegisterServiceLocatorMessages(Store(m_clientProtocol->GetSlots()));
  RegisterRegistryServices(Store(m_clientProtocol->GetSlots()));
  std::unique_ptr<VirtualServiceLocatorClient> registryServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  registryServiceLocatorClient->SetCredentials("root", "");
  registryServiceLocatorClient->Open();
  m_container.Initialize(Initialize(std::move(registryServiceLocatorClient),
    Initialize(m_dataStore)), &*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
}

void RegistryServletTester::tearDown() {
  m_clientProtocol.Reset();
  m_container.Reset();
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
