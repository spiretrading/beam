#ifndef BEAM_REGISTRY_SERVICE_TEST_ENVIRONMENT_HPP
#define BEAM_REGISTRY_SERVICE_TEST_ENVIRONMENT_HPP
#include <boost/functional/factory.hpp>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/RegistryService/RegistryClient.hpp"
#include "Beam/RegistryService/RegistryServlet.hpp"
#include "Beam/RegistryService/LocalRegistryDataStore.hpp"
#include "Beam/RegistryService/VirtualRegistryClient.hpp"
#include "Beam/RegistryServiceTests/RegistryServiceTests.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

namespace Beam::RegistryService::Tests {

  /**
   * Wraps most components needed to run an instance of the RegistryService with
   * helper functions.
   */
  class RegistryServiceTestEnvironment {
    public:

      /**
       * Constructs a RegistryServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       */
      RegistryServiceTestEnvironment(
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      ~RegistryServiceTestEnvironment();

      /**
       * Builds a new RegistryClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the RegistryClient.
       */
      std::unique_ptr<VirtualRegistryClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      void Close();

    private:
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;
      using ServiceLocatorClient = ServiceLocator::VirtualServiceLocatorClient;
      using ServiceProtocolServletContainer =
        Services::ServiceProtocolServletContainer<
        ServiceLocator::MetaAuthenticationServletAdapter<MetaRegistryServlet<
        LocalRegistryDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*, Serialization::BinarySender<IO::SharedBuffer>,
        Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;
      LocalRegistryDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;

      RegistryServiceTestEnvironment(
        const RegistryServiceTestEnvironment&) = delete;
      RegistryServiceTestEnvironment& operator =(
        const RegistryServiceTestEnvironment&) = delete;
  };

  inline RegistryServiceTestEnvironment::RegistryServiceTestEnvironment(
    std::shared_ptr<ServiceLocator::VirtualServiceLocatorClient>
    serviceLocatorClient)
    : m_container(Beam::Initialize(serviceLocatorClient, Beam::Initialize(
        &m_dataStore)), &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline RegistryServiceTestEnvironment::~RegistryServiceTestEnvironment() {
    Close();
  }

  inline std::unique_ptr<VirtualRegistryClient>
      RegistryServiceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    auto builder = ServiceProtocolClientBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_registry_client", m_serverConnection);
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<RegistryClient<
      ServiceProtocolClientBuilder>>(builder);
    return MakeVirtualRegistryClient(std::move(client));
  }

  inline void RegistryServiceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
