#ifndef BEAM_LOCALSERVERCONNECTIONTESTER_HPP
#define BEAM_LOCALSERVERCONNECTIONTESTER_HPP
#include <optional>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {
namespace Tests {

  /*! \class LocalServerConnectionTester
      \brief Tests the suite of LocalServerConnection classes.
   */
  class LocalServerConnectionTester : public CPPUNIT_NS::TestFixture {
    public:
      typedef LocalServerConnection<SharedBuffer> ServerConnection;
      typedef LocalClientChannel<SharedBuffer> ClientChannel;

      virtual void setUp();

      virtual void tearDown();

      //! Tests an Accept operation but then closing the server.
      void TestAcceptThenClose();

      //! Tests accepting a Channel that's already pending.
      void TestAcceptPendingChannel();

      //! Tests accepting a Channel that has not yet been instantiated.
      void TestAcceptFutureChannel();

      //! Tests waiting to accept a Channel that has not yet been instantiated.
      void TestFutureAcceptOfFutureChannel();

      //! Tests opening a Channel before the ServerConnection is open.
      void TestOpenClientBeforeOpenServer();

      //! Tests opening a Channel and then closing the ServerConnection.
      void TestOpenClientThenCloseServer();

      //! Tests opening a Channel after the the ServerConnection has closed.
      void TestOpenClientAfterCloseServer();

    private:
      std::optional<ServerConnection> m_serverConnection;

      CPPUNIT_TEST_SUITE(LocalServerConnectionTester);
        CPPUNIT_TEST(TestAcceptThenClose);
        CPPUNIT_TEST(TestAcceptPendingChannel);
        CPPUNIT_TEST(TestAcceptFutureChannel);
        CPPUNIT_TEST(TestFutureAcceptOfFutureChannel);
        CPPUNIT_TEST(TestOpenClientBeforeOpenServer);
        CPPUNIT_TEST(TestOpenClientThenCloseServer);
        CPPUNIT_TEST(TestOpenClientAfterCloseServer);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
