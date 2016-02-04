#ifndef BEAM_HTTPREQUESTPARSERTESTER_HPP
#define BEAM_HTTPREQUESTPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/WebServicesTests/WebServicesTests.hpp"

namespace Beam {
namespace WebServices {
namespace Tests {

  /*  \class HttpRequestParserTester
      \brief Tests the HttpRequestParser class.
   */
  class HttpRequestParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing a valid request.
      void TestValidRequest();

    private:
      CPPUNIT_TEST_SUITE(HttpRequestParserTester);
        CPPUNIT_TEST(TestValidRequest);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
