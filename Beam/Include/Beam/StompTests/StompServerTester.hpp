#ifndef BEAM_STOMPSERVERTESTER_HPP
#define BEAM_STOMPSERVERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Stomp/StompServer.hpp"
#include "Beam/StompTests/StompTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Stomp {
namespace Tests {

  /*! \class StompServerTester
      \brief Tests the StompServer class.
   */
  class StompServerTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of server to connect to.
      using ServerConnection =  IO::LocalServerConnection<IO::SharedBuffer>;

      //! The server side Channel.
      using ServerChannel = IO::LocalServerChannel<IO::SharedBuffer>;

      //! The client side Channel.
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;

      //! The type of StompServer to test.
      using TestStompServer = StompServer<std::unique_ptr<ServerChannel>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests receiving a CONNECT command.
      void TestReceivingConnectCommand();

    private:
      boost::optional<ServerConnection> m_serverConnection;
      boost::optional<ClientChannel> m_clientChannel;
      boost::optional<TestStompServer> m_server;

      CPPUNIT_TEST_SUITE(StompServerTester);
        CPPUNIT_TEST(TestReceivingConnectCommand);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
