#ifndef AVALON_HTTPSERVERTESTER_HPP
#define AVALON_HTTPSERVERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Avalon/IOTests/MockClientChannel.hpp"
#include "Avalon/SignalHandling/SignalSink.hpp"
#include "Avalon/WebServices/HttpServer.hpp"
#include "Avalon/WebServicesTests/WebServicesTests.hpp"

namespace Avalon {
namespace WebServices {
namespace Tests {

  /*  \class HttpServerTester
      \brief Tests the HttpServer class.
   */
  class HttpServerTester : public CPPUNIT_NS::TestFixture {
    public:
      virtual void setUp();

      virtual void tearDown();

      //! Tests an HTTP header in two writes.
      void TestHeaderInTwoWrites();

      //! Tests an HTTP request sent via two feeds/writes.
      void TestRequestInTwoWrites();

    private:
      IO::Tests::MockServerConnection* m_serverConnection;
      boost::scoped_ptr<HttpServer> m_server;
      boost::scoped_ptr<IO::Tests::MockClientChannel> m_channel;
      SignalHandling::SignalSink m_sessionSink;

      CPPUNIT_TEST_SUITE(HttpServerTester);
        CPPUNIT_TEST(TestHeaderInTwoWrites);
        CPPUNIT_TEST(TestRequestInTwoWrites);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif // AVALON_HTTPSERVERTESTER_HPP
