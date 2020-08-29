#ifndef BEAM_UIDSERVICETESTENVIRONMENT_HPP
#define BEAM_UIDSERVICETESTENVIRONMENT_HPP
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
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
#include "Beam/UidService/UidServlet.hpp"
#include "Beam/UidService/VirtualUidClient.hpp"
#include "Beam/UidServiceTests/UidServiceTests.hpp"

namespace Beam {
namespace UidService {
namespace Tests {

  /*! \class UidServiceTestEnvironment
      \brief Wraps most components needed to run an instance of the
             UidService with helper functions.
   */
  class UidServiceTestEnvironment : private boost::noncopyable {
    public:

      //! Constructs a UidServiceTestEnvironment.
      UidServiceTestEnvironment();

      ~UidServiceTestEnvironment();

      //! Closes the servlet.
      void Close();

      //! Builds a new UidClient.
      std::unique_ptr<VirtualUidClient> BuildClient();

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

  inline std::unique_ptr<VirtualUidClient>
      UidServiceTestEnvironment::BuildClient() {
    ServiceProtocolClientBuilder builder(
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_uid_client", m_serverConnection);
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<UidService::UidClient<
      ServiceProtocolClientBuilder>>(builder);
    return MakeVirtualUidClient(std::move(client));
  }
}
}
}

#endif
