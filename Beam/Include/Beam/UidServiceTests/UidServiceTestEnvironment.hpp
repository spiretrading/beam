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
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/UidClientBox.hpp"
#include "Beam/UidService/UidServlet.hpp"
#include "Beam/UidServiceTests/UidServiceTests.hpp"

namespace Beam::UidService::Tests {

  /**
   * Wraps most components needed to run an instance of the UidService with
   * helper functions.
   */
  class UidServiceTestEnvironment {
    public:

      /** Constructs a UidServiceTestEnvironment. */
      UidServiceTestEnvironment();

      ~UidServiceTestEnvironment();

      UidClientBox MakeClient();

      void Close();

    private:
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Services::ServiceProtocolServletContainer<
          MetaUidServlet<LocalUidDataStore*>, ServerConnection*,
          Serialization::BinarySender<IO::SharedBuffer>,
          Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Services::ServiceProtocolClientBuilder<
          Services::MessageProtocol<std::unique_ptr<ClientChannel>,
            Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
          Threading::TriggerTimer>;
      LocalUidDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;

      UidServiceTestEnvironment(const UidServiceTestEnvironment&) = delete;
      UidServiceTestEnvironment& operator =(
        const UidServiceTestEnvironment&) = delete;
  };

  inline UidServiceTestEnvironment::UidServiceTestEnvironment()
    : m_container(&m_dataStore, &m_serverConnection,
        boost::factory<std::shared_ptr<Threading::TriggerTimer>>()) {}

  inline UidServiceTestEnvironment::~UidServiceTestEnvironment() {
    Close();
  }

  inline void UidServiceTestEnvironment::Close() {
    m_container.Close();
  }

  inline UidClientBox UidServiceTestEnvironment::MakeClient() {
    return UidClientBox(
      std::in_place_type<UidClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::bind_front(boost::factory<
        std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
        "test_uid_client", std::ref(m_serverConnection)), std::bind_front(
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>())));
  }
}

#endif
