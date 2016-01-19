#ifndef BEAM_SERVICELOCATORTESTINSTANCE_HPP
#define BEAM_SERVICELOCATORTESTINSTANCE_HPP
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTests.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Tests {

  /*! \class ServiceLocatorTestInstance
      \brief Wraps most components needed to run an instance of the
             ServiceLocator with helper functions.
   */
  class ServiceLocatorTestInstance : private boost::noncopyable {
    public:

      //! The type of ServerConnection.
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServletContainer =
        Services::ServiceProtocolServletContainer<
        MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>,
        ServerConnection*, Serialization::BinarySender<IO::SharedBuffer>,
        Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;

      //! The type used to build ServiceLocatorClient sessions.
      using ServiceProtocolClientBuilder =
        Services::ServiceProtocolClientBuilder<Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;

      //! The type of ServiceLocatorClient used.
      using ServiceLocatorClient = ServiceLocator::ServiceLocatorClient<
        ServiceProtocolClientBuilder>;

      //! Constructs a ServiceLocatorTestInstance.
      ServiceLocatorTestInstance();

      ~ServiceLocatorTestInstance();

      //! Opens the servlet.
      void Open();

      //! Closes the servlet.
      void Close();

      //! Returns a ServiceLocatorClient logged in as the root account.
      ServiceLocatorClient& GetRoot();

      //! Builds a new ServiceLocatorClient.
      std::unique_ptr<ServiceLocatorClient> BuildClient();

    private:
      LocalServiceLocatorDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;
      std::unique_ptr<ServiceLocatorClient> m_root;
  };

  inline ServiceLocatorTestInstance::ServiceLocatorTestInstance()
      : m_container(&m_dataStore, &m_serverConnection,
          boost::factory<std::shared_ptr<Threading::TriggerTimer>>()) {}

  inline ServiceLocatorTestInstance::~ServiceLocatorTestInstance() {
    Close();
  }

  inline void ServiceLocatorTestInstance::Open() {
    m_container.Open();
    m_root = BuildClient();
    m_root->SetCredentials("root", "");
    m_root->Open();
  }

  inline void ServiceLocatorTestInstance::Close() {
    m_root.reset();
    m_container.Close();
  }

  inline ServiceLocatorTestInstance::ServiceLocatorClient&
      ServiceLocatorTestInstance::GetRoot() {
    return *m_root;
  }

  inline std::unique_ptr<ServiceLocatorTestInstance::ServiceLocatorClient>
      ServiceLocatorTestInstance::BuildClient() {
    ServiceProtocolClientBuilder builder(
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_service_locator_client", Ref(m_serverConnection));
      },
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<ServiceLocatorClient>(builder);
    return client;
  }
}
}
}

#endif
