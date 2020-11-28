#ifndef BEAM_REGISTRY_SERVICE_TEST_ENVIRONMENT_HPP
#define BEAM_REGISTRY_SERVICE_TEST_ENVIRONMENT_HPP
#include <boost/functional/factory.hpp>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/RegistryService/RegistryClient.hpp"
#include "Beam/RegistryService/RegistryClientBox.hpp"
#include "Beam/RegistryService/RegistryServlet.hpp"
#include "Beam/RegistryService/LocalRegistryDataStore.hpp"
#include "Beam/RegistryServiceTests/RegistryServiceTests.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClientBox.hpp"
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
       * @param serviceLocatorClient The ServiceLocatorClientBox to use.
       */
      RegistryServiceTestEnvironment(
        ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      ~RegistryServiceTestEnvironment();

      /**
       * Returns a new RegistryClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the RegistryClient.
       */
      RegistryClientBox MakeClient(
        ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      void Close();

    private:
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Services::ServiceProtocolServletContainer<
          ServiceLocator::MetaAuthenticationServletAdapter<MetaRegistryServlet<
            LocalRegistryDataStore*>, ServiceLocator::ServiceLocatorClientBox>,
          ServerConnection*, Serialization::BinarySender<IO::SharedBuffer>,
          Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Services::AuthenticatedServiceProtocolClientBuilder<
          ServiceLocator::ServiceLocatorClientBox, Services::MessageProtocol<
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
    ServiceLocator::ServiceLocatorClientBox serviceLocatorClient)
    : m_container(Initialize(std::move(serviceLocatorClient),
        Initialize(&m_dataStore)), &m_serverConnection,
        boost::factory<std::shared_ptr<Threading::TriggerTimer>>()) {}

  inline RegistryServiceTestEnvironment::~RegistryServiceTestEnvironment() {
    Close();
  }

  inline RegistryClientBox RegistryServiceTestEnvironment::MakeClient(
      ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return RegistryClientBox(
      std::in_place_type<RegistryClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
        std::bind(boost::factory<std::unique_ptr<
          ServiceProtocolClientBuilder::Channel>>(), "test_registry_client",
          std::ref(m_serverConnection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void RegistryServiceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
