#ifndef BEAM_SERVICELOCATORTESTENVIRONMENT_HPP
#define BEAM_SERVICELOCATORTESTENVIRONMENT_HPP
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
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
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTests.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Tests {

  /*! \class ServiceLocatorTestEnvironment
      \brief Wraps most components needed to run an instance of the
             ServiceLocator with helper functions.
   */
  class ServiceLocatorTestEnvironment : private boost::noncopyable {
    public:

      //! Constructs a ServiceLocatorTestEnvironment.
      ServiceLocatorTestEnvironment();

      ~ServiceLocatorTestEnvironment();

      //! Opens the servlet.
      void Open();

      //! Closes the servlet.
      void Close();

      //! Returns a ServiceLocatorClient logged in as the root account.
      VirtualServiceLocatorClient& GetRoot();

      //! Builds a new ServiceLocatorClient.
      std::unique_ptr<VirtualServiceLocatorClient> BuildClient();

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
      std::unique_ptr<VirtualServiceLocatorClient> m_root;
  };

  inline ServiceLocatorTestEnvironment::ServiceLocatorTestEnvironment()
      : m_container(&m_dataStore, &m_serverConnection,
          boost::factory<std::shared_ptr<Threading::TriggerTimer>>()) {}

  inline ServiceLocatorTestEnvironment::~ServiceLocatorTestEnvironment() {
    Close();
  }

  inline void ServiceLocatorTestEnvironment::Open() {
    m_container.Open();
    m_root = BuildClient();
    m_root->SetCredentials("root", "");
    m_root->Open();
  }

  inline void ServiceLocatorTestEnvironment::Close() {
    m_root.reset();
    m_container.Close();
  }

  inline VirtualServiceLocatorClient& ServiceLocatorTestEnvironment::GetRoot() {
    return *m_root;
  }

  inline std::unique_ptr<VirtualServiceLocatorClient>
      ServiceLocatorTestEnvironment::BuildClient() {
    ServiceProtocolClientBuilder builder(
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_service_locator_client", Ref(m_serverConnection));
      },
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<ServiceLocator::ServiceLocatorClient<
      ServiceProtocolClientBuilder>>(builder);
    return MakeVirtualServiceLocatorClient(std::move(client));
  }
}
}
}

#endif
