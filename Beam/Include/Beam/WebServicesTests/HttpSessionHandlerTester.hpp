#ifndef AVALON_HTTPSESSIONHANDLERTESTER_HPP
#define AVALON_HTTPSESSIONHANDLERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Avalon/IOTests/MockClientChannel.hpp"
#include "Avalon/SignalHandling/SignalSink.hpp"
#include "Avalon/WebServices/HttpServer.hpp"
#include "Avalon/WebServices/HttpSessionHandler.hpp"
#include "Avalon/WebServicesTests/WebServicesTests.hpp"

namespace Avalon {
namespace WebServices {
namespace Tests {

  /*  \class HttpSessionHandlerTester
      \brief Tests the HttpSessionHandler class.
   */
  class HttpSessionHandlerTester : public CPPUNIT_NS::TestFixture {
    public:

      virtual void setUp();

      virtual void tearDown();

      //! Tests a basic session run through.
      void TestValidSession();

    private:
      IO::Tests::MockServerConnection* m_serverConnection;
      boost::scoped_ptr<HttpServer> m_server;
      boost::scoped_ptr<HttpSessionHandler> m_sessionHandler;
      SignalHandling::SignalSink m_sessionSink;
      boost::scoped_ptr<IO::Tests::MockClientChannel> m_channel;

      CPPUNIT_TEST_SUITE(HttpSessionHandlerTester);
        CPPUNIT_TEST(TestValidSession);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif // AVALON_HTTPSESSIONHANDLERTESTER_HPP
