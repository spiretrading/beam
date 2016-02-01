#ifndef AVALON_WEBSERVLETCONTAINERTESTER_HPP
#define AVALON_WEBSERVLETCONTAINERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Avalon/IOTests/MockServerConnection.hpp"
#include "Avalon/Services/RecordMessage.hpp"
#include "Avalon/Services/Service.hpp"
#include "Avalon/Services/ServiceProtocolServerChannel.hpp"
#include "Avalon/Services/ServiceProtocolServlet.hpp"
#include "Avalon/SignalHandling/SignalSink.hpp"
#include "Avalon/WebServices/HttpServer.hpp"
#include "Avalon/WebServices/HttpSessionHandler.hpp"
#include "Avalon/WebServices/WebServletContainer.hpp"
#include "Avalon/WebServicesTests/WebServicesTests.hpp"

namespace Avalon {
namespace WebServices {
namespace Tests {
  AVALON_DECLARE_SERVICE(EchoService, "EchoService", int, int, value);

  /*  \class WebServletContainerTester
      \brief Tests the WebServletContainer class.
   */
  class WebServletContainerTester : public CPPUNIT_NS::TestFixture {
    public:

      virtual void setUp();

      virtual void tearDown();

      void TestRequest();

    private:
      class TestChannel : public IO::Channel,
          public Services::ServiceProtocolServerChannel {
        public:
          TestChannel(IO::Channel* channel);

          virtual ~TestChannel();

          IO::Connection& GetConnection();

          IO::Reader& GetReader();

          IO::Writer& GetWriter();

        private:
          boost::scoped_ptr<IO::Channel> m_channel;
      };
      class TestServlet : public Services::ServiceProtocolServlet<
          TestServlet, TestChannel> {
        public:
          void Initialize(const Initializer<TestServlet>& initializer);

        private:
          void OnEchoRequest(const Services::RequestToken<EchoService>& request,
            int value);
      };
      IO::Tests::MockServerConnection* m_serverConnection;
      SignalHandling::SignalSink m_sessionSink;
      boost::scoped_ptr<IO::Tests::MockClientChannel> m_channel;
      boost::scoped_ptr<WebServletContainer<TestServlet, TestChannel> >
        m_container;

      CPPUNIT_TEST_SUITE(WebServletContainerTester);
        CPPUNIT_TEST(TestRequest);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif // AVALON_WEBSERVLETCONTAINERTESTER_HPP
