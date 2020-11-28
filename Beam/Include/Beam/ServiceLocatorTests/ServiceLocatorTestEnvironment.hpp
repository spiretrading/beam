#ifndef BEAM_SERVICE_LOCATOR_TEST_ENVIRONMENT_HPP
#define BEAM_SERVICE_LOCATOR_TEST_ENVIRONMENT_HPP
#include <boost/functional/factory.hpp>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClientBox.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTests.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

namespace Beam::ServiceLocator::Tests {

  /**
   * Wraps most components needed to run an instance of the ServiceLocator with
   * helper functions.
   */
  class ServiceLocatorTestEnvironment {
    public:

      /** Constructs a ServiceLocatorTestEnvironment. */
      ServiceLocatorTestEnvironment();

      ~ServiceLocatorTestEnvironment();

      /** Closes the servlet. */
      void Close();

      /** Returns a ServiceLocatorClientBox logged in as the root account. */
      ServiceLocatorClientBox& GetRoot();

      /** Makes a new ServiceLocatorClientBox. */
      ServiceLocatorClientBox MakeClient(std::string username,
        std::string password);

      /** Makes a new ServiceLocatorClient. */
      ServiceLocatorClientBox MakeClient();

    private:
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Services::ServiceProtocolServletContainer<
        MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>,
        ServerConnection*, Serialization::BinarySender<IO::SharedBuffer>,
        Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Services::ServiceProtocolClientBuilder<Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;
      LocalServiceLocatorDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;
      ServiceLocatorClientBox m_root;

      ServiceLocatorTestEnvironment(
        const ServiceLocatorTestEnvironment&) = delete;
      ServiceLocatorTestEnvironment& operator =(
        const ServiceLocatorTestEnvironment&) = delete;
  };

  inline ServiceLocatorTestEnvironment::ServiceLocatorTestEnvironment()
    : m_container(&m_dataStore, &m_serverConnection,
        boost::factory<std::shared_ptr<Threading::TriggerTimer>>()),
      m_root(MakeClient("root", "")) {}

  inline ServiceLocatorTestEnvironment::~ServiceLocatorTestEnvironment() {
    Close();
  }

  inline void ServiceLocatorTestEnvironment::Close() {
    m_root.Close();
    m_container.Close();
  }

  inline ServiceLocatorClientBox& ServiceLocatorTestEnvironment::GetRoot() {
    return m_root;
  }

  inline ServiceLocatorClientBox ServiceLocatorTestEnvironment::MakeClient(
      std::string username, std::string password) {
    return ServiceLocatorClientBox(
      std::in_place_type<ServiceLocatorClient<ServiceProtocolClientBuilder>>,
      std::move(username), std::move(password), ServiceProtocolClientBuilder(
        std::bind(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_service_locator_client", std::ref(m_serverConnection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline ServiceLocatorClientBox ServiceLocatorTestEnvironment::MakeClient() {
    return MakeClient("root", "");
  }
}

#endif
