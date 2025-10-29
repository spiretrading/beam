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
#include "Beam/ServiceLocator/ProtocolServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

namespace Beam::Tests {

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
      void close();

      /** Returns a ServiceLocatorClient logged in as the root account. */
      ServiceLocatorClient& get_root();

      /** Makes a new ServiceLocatorClient. */
      ServiceLocatorClient make_client(
        std::string username, std::string password);

      /** Makes a new ServiceLocatorClient. */
      ServiceLocatorClient make_client();

    private:
      using ServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>,
          LocalServerConnection*, BinarySender<SharedBuffer>, NullEncoder,
          std::shared_ptr<TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::ServiceProtocolClientBuilder<MessageProtocol<
          std::unique_ptr<LocalClientChannel>, BinarySender<SharedBuffer>,
          NullEncoder>, TriggerTimer>;
      LocalServiceLocatorDataStore m_data_store;
      LocalServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;
      ServiceLocatorClient m_root;

      ServiceLocatorTestEnvironment(
        const ServiceLocatorTestEnvironment&) = delete;
      ServiceLocatorTestEnvironment& operator =(
        const ServiceLocatorTestEnvironment&) = delete;
  };

  inline ServiceLocatorTestEnvironment::ServiceLocatorTestEnvironment()
    : m_container(&m_data_store, &m_server_connection,
        boost::factory<std::shared_ptr<TriggerTimer>>()),
      m_root(make_client("root", "")) {}

  inline ServiceLocatorTestEnvironment::~ServiceLocatorTestEnvironment() {
    close();
  }

  inline void ServiceLocatorTestEnvironment::close() {
    m_root.close();
    m_container.close();
  }

  inline ServiceLocatorClient& ServiceLocatorTestEnvironment::get_root() {
    return m_root;
  }

  inline ServiceLocatorClient ServiceLocatorTestEnvironment::make_client(
      std::string username, std::string password) {
    return ServiceLocatorClient(std::in_place_type<
      ProtocolServiceLocatorClient<ServiceProtocolClientBuilder>>,
      std::move(username), std::move(password), ServiceProtocolClientBuilder(
        std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_service_locator_client", std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline ServiceLocatorClient ServiceLocatorTestEnvironment::make_client() {
    return make_client("root", "");
  }
}

#endif
