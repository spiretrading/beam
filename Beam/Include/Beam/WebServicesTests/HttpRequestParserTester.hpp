#ifndef AVALON_HTTPREQUESTPARSERTESTER_HPP
#define AVALON_HTTPREQUESTPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Avalon/WebServicesTests/WebServicesTests.hpp"

namespace Avalon {
namespace WebServices {
namespace Tests {

  /*  \class HttpClientChannelTester
      \brief Tests the HttpRequestParser class.
   */
  class HttpRequestParserTester : public CPPUNIT_NS::TestFixture {
    public:
      virtual void setUp();

      virtual void tearDown();

      //! Tests parsing an HTTP request.
      void TestParsingRequest();

      //! Tests parsing an HTTP header in two writes.
      void TestParsingHeaderInTwoWrites();

      //! Tests parsing an HTTP request sent via two feeds/writes.
      void TestParsingRequestInTwoWrites();

      //! Tests parsing an HTTP body sent via two feeds/writes.
      void TestParsingBodyInTwoWrites();

    private:
      CPPUNIT_TEST_SUITE(HttpRequestParserTester);
        CPPUNIT_TEST(TestParsingRequest);
        CPPUNIT_TEST(TestParsingHeaderInTwoWrites);
        CPPUNIT_TEST(TestParsingRequestInTwoWrites);
        CPPUNIT_TEST(TestParsingBodyInTwoWrites);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif // AVALON_HTTPREQUESTPARSERTESTER_HPP
