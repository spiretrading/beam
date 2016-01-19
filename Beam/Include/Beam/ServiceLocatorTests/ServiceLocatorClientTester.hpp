#ifndef BEAM_SERVICELOCATORCLIENTTESTER_HPP
#define BEAM_SERVICELOCATORCLIENTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolServer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Tests {

  /*! \class ServiceLocatorClientTester
      \brief Tests the ServiceLocatorClient class.
   */
  class ServiceLocatorClientTester : public CPPUNIT_NS::TestFixture {
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
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;

      //! The type of ServiceLocatorClient.
      using TestServiceLocatorClient = ServiceLocatorClient<
        ServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Test logging in and getting accepted.
      void TestLoginAccepted();

      //! Test logging in and getting rejected.
      void TestLoginRejected();

      //! Tests monitoring a DirectoryEntry.
      void TestMonitorDirectoryEntry();

    private:
      DelayPtr<ServerConnection> m_serverConnection;
      DelayPtr<ServiceProtocolServer> m_protocolServer;
      DelayPtr<TestServiceLocatorClient> m_serviceClient;

      CPPUNIT_TEST_SUITE(ServiceLocatorClientTester);
        CPPUNIT_TEST(TestLoginAccepted);
        CPPUNIT_TEST(TestLoginRejected);
        CPPUNIT_TEST(TestMonitorDirectoryEntry);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
