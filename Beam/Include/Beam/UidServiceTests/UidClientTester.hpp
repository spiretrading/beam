#ifndef BEAM_UIDCLIENTTESTER_HPP
#define BEAM_UIDCLIENTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolServer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace UidService {
namespace Tests {

  /*! \class UidClientTester
      \brief Tests the UidClient class.
   */
  class UidClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      typedef IO::LocalServerConnection<IO::SharedBuffer> ServerConnection;

      //! The type of Channel from the client to the server.
      typedef IO::LocalClientChannel<IO::SharedBuffer> ClientChannel;

      //! The type of ServiceProtocolServer.
      typedef Services::ServiceProtocolServer<ServerConnection*,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder,
        std::shared_ptr<Threading::TriggerTimer>> ServiceProtocolServer;

      //! The type used to build sessions.
      typedef Services::ServiceProtocolClientBuilder<
        Services::MessageProtocol<std::unique_ptr<ClientChannel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer> ServiceProtocolClientBuilder;

      //! The type of UidClient.
      typedef UidClient<ServiceProtocolClientBuilder> TestUidClient;

      virtual void setUp();

      virtual void tearDown();

      //! Tests requesting a single UID.
      void TestSingleUidRequest();

      //! Tests requesting multiple UIDs one after another.
      void TestSequentialUidRequests();

      //! Tests requesting multiple UIDs before the server gets a chance to
      //! service the first request.
      void TestSimultaneousUidRequests();

      //! Tests requesting so many UIDs that it requires two server-side
      //! requests to satisfy.
      void TestMultipleServerRequests();

    private:
      DelayPtr<ServerConnection> m_serverConnection;
      DelayPtr<ServiceProtocolServer> m_protocolServer;
      DelayPtr<TestUidClient> m_uidClient;

      CPPUNIT_TEST_SUITE(UidClientTester);
        CPPUNIT_TEST(TestSingleUidRequest);
        CPPUNIT_TEST(TestSequentialUidRequests);
        CPPUNIT_TEST(TestSimultaneousUidRequests);
        CPPUNIT_TEST(TestMultipleServerRequests);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
