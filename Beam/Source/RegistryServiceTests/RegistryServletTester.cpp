#include <boost/functional/factory.hpp>
#include <boost/optional.hpp>
#include <doctest/doctest.h>
#include "Beam/RegistryService/LocalRegistryDataStore.hpp"
#include "Beam/RegistryService/RegistryServlet.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::RegistryService;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaRegistryServlet<LocalRegistryDataStore*>>;
    ServiceLocatorTestEnvironment m_environment;
    LocalRegistryDataStore m_dataStore;
    optional<ServletContainer> m_container;
    optional<TestServiceProtocolClient> m_clientProtocol;

    Fixture() {
      auto registryServiceLocatorClient = m_environment.BuildClient();
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_container.emplace(Initialize(std::move(registryServiceLocatorClient),
        Initialize(&m_dataStore)), serverConnection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_clientProtocol.emplace(Initialize("test", *serverConnection),
        Initialize());
      RegisterServiceLocatorServices(Store(m_clientProtocol->GetSlots()));
      RegisterServiceLocatorMessages(Store(m_clientProtocol->GetSlots()));
      RegisterRegistryServices(Store(m_clientProtocol->GetSlots()));
    }
  };
}

TEST_SUITE("RegistryServlet") {
  TEST_CASE_FIXTURE(Fixture, "make_directory") {
    auto serviceLocatorClient = m_environment.BuildClient();
    OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
      Ref(*serviceLocatorClient)), *m_clientProtocol);
    auto directoryName = std::string("directory");
    auto directory = m_clientProtocol->SendRequest<
      RegistryService::MakeDirectoryService>(directoryName,
      RegistryEntry::GetRoot());
  }

  TEST_CASE_FIXTURE(Fixture, "make_value") {
    auto serviceLocatorClient = m_environment.BuildClient();
    OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
      Ref(*serviceLocatorClient)), *m_clientProtocol);
    auto key = std::string("key");
    auto value = BufferFromString<SharedBuffer>("value");
    auto entry = m_clientProtocol->SendRequest<MakeValueService>(key, value,
      RegistryEntry::GetRoot());
  }

  TEST_CASE_FIXTURE(Fixture, "load_path") {
    auto serviceLocatorClient = m_environment.BuildClient();
    OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
      Ref(*serviceLocatorClient)), *m_clientProtocol);
    auto directory = RegistryEntry(RegistryEntry::Type::DIRECTORY, 0,
      "directory", 0);
    directory = m_dataStore.Copy(directory, RegistryEntry::GetRoot());
    auto pathLoaded = m_clientProtocol->SendRequest<
      RegistryService::LoadPathService>(RegistryEntry::GetRoot(), "directory");
    REQUIRE(pathLoaded == directory);
  }

  TEST_CASE_FIXTURE(Fixture, "load_value") {
    auto serviceLocatorClient = m_environment.BuildClient();
    OpenAndAuthenticate(SessionAuthenticator<VirtualServiceLocatorClient>(
      Ref(*serviceLocatorClient)), *m_clientProtocol);
    auto valueEntry = RegistryEntry(RegistryEntry::Type::VALUE, 0, "value", 0);
    valueEntry = m_dataStore.Copy(valueEntry, RegistryEntry::GetRoot());
    auto value = BufferFromString<SharedBuffer>("value");
    valueEntry = m_dataStore.Store(valueEntry, value);
    auto loadedValue = m_clientProtocol->SendRequest<LoadValueService>(
      valueEntry);
    REQUIRE(loadedValue == value);
  }
}
