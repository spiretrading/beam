#ifndef BEAM_UID_SERVICE_TEST_ENVIRONMENT_HPP
#define BEAM_UID_SERVICE_TEST_ENVIRONMENT_HPP
#include <boost/functional/factory.hpp>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidService/ServiceUidClient.hpp"
#include "Beam/UidService/UidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"

namespace Beam::Tests {

  /**
   * Wraps most components needed to run an instance of the UidService with
   * helper functions.
   */
  class UidServiceTestEnvironment {
    public:

      /** Constructs a UidServiceTestEnvironment. */
      UidServiceTestEnvironment();

      ~UidServiceTestEnvironment();

      /** Makes a UidClient connected to the UidService. */
      UidClient make_client();

      void close();

    private:
      using ServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          MetaUidServlet<LocalUidDataStore*>, LocalServerConnection*,
          BinarySender<SharedBuffer>, NullEncoder,
          std::shared_ptr<TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::ServiceProtocolClientBuilder<
          MessageProtocol<std::unique_ptr<LocalClientChannel>,
            BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
      LocalUidDataStore m_data_store;
      LocalServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;

      UidServiceTestEnvironment(const UidServiceTestEnvironment&) = delete;
      UidServiceTestEnvironment& operator =(
        const UidServiceTestEnvironment&) = delete;
  };

  inline UidServiceTestEnvironment::UidServiceTestEnvironment()
    : m_container(&m_data_store, &m_server_connection,
        boost::factory<std::shared_ptr<TriggerTimer>>()) {}

  inline UidServiceTestEnvironment::~UidServiceTestEnvironment() {
    close();
  }

  inline void UidServiceTestEnvironment::close() {
    m_container.close();
  }

  inline UidClient UidServiceTestEnvironment::make_client() {
    return UidClient(
      std::in_place_type<ServiceUidClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::bind_front(boost::factory<
        std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
        "test_uid_client", std::ref(m_server_connection)), std::bind_front(
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>())));
  }
}

#endif
