#ifndef BEAM_AUTHENTICATIONSERVLETADAPTERTESTER_HPP
#define BEAM_AUTHENTICATIONSERVLETADAPTERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/ServicesTests/TestServlet.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Tests {

  /*! \class AuthenticationServletAdapterTester
      \brief Tests the AuthenticationServletAdapter class.
   */
  class AuthenticationServletAdapterTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer = Services::ServiceProtocolServer<
        ServerConnection*, Serialization::BinarySender<IO::SharedBuffer>,
        Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;

      //! The type used to build sessions.
      using ServiceProtocolClientBuilder =
        Services::ServiceProtocolClientBuilder<Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Serialization::BinarySender<IO::SharedBuffer>,
        Codecs::NullEncoder>, Threading::TriggerTimer>;

      //! The type of ServiceLocatorClient.
      using TestServiceLocatorClient =
        ServiceLocatorClient<ServiceProtocolClientBuilder>;

      //! The result of adapting the TestServlet with the
      //! AuthenticationServletAdapter.
      using MetaServlet = MetaAuthenticationServletAdapter<
        Services::Tests::MetaTestServlet,
        std::unique_ptr<TestServiceLocatorClient>>;

      //! The type of ServiceProtocolServletContainer.
      using ServiceProtocolServletContainer =
        Services::ServiceProtocolServletContainer<MetaServlet,
        std::unique_ptr<ServerConnection>, Serialization::BinarySender<
        IO::SharedBuffer>, Codecs::NullEncoder,
        std::shared_ptr<Threading::TriggerTimer>>;

      //! The type of Servlet.
      using Servlet = AuthenticationServletAdapter<
        ServiceProtocolServletContainer,
        Services::Tests::TestServlet<ServiceProtocolServletContainer>,
        std::unique_ptr<TestServiceLocatorClient>>;

      //! The type of ServiceProtocolClient on the client side.
      using ClientServiceProtocolClient = Services::ServiceProtocolClient<
        Services::MessageProtocol<ClientChannel,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;

      //! The type of ServiceProtocolServer.
      using ServiceLocatorContainer = Services::ServiceProtocolServletContainer<
        MetaServiceLocatorServlet<
        std::shared_ptr<LocalServiceLocatorDataStore>>,
        std::unique_ptr<ServerConnection>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder,
        std::shared_ptr<Threading::TriggerTimer>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests requesting a service without authentication.
      void TestServiceWithoutAuthentication();

      //! Tests requesting a service with authentication.
      void TestServiceWithAuthentication();

    private:
      std::shared_ptr<LocalServiceLocatorDataStore> m_dataStore;
      DelayPtr<ServiceProtocolServletContainer> m_container;
      DelayPtr<ClientServiceProtocolClient> m_clientProtocol;
      DelayPtr<ServiceLocatorContainer> m_serviceLocatorContainer;
      DelayPtr<TestServiceLocatorClient> m_serviceLocatorClient;

      CPPUNIT_TEST_SUITE(AuthenticationServletAdapterTester);
        CPPUNIT_TEST(TestServiceWithoutAuthentication);
        CPPUNIT_TEST(TestServiceWithAuthentication);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
