#ifndef BEAM_URITESTER_HPP
#define BEAM_URITESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/WebServicesTests/WebServicesTests.hpp"

namespace Beam {
namespace WebServices {
namespace Tests {

  /*  \class UriTester
      \brief Tests the Uri class.
   */
  class UriTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing a scheme only.
      void TestSchemeOnly();

      //! Tests parsing a hostname without a scheme.
      void TestHostnameOnly();

      //! Tests parsing a scheme and an empty hostname.
      void TestSchemeAndEmptyHostname();

      //! Tests parsing a scheme and hostname.
      void TestSchemeAndHostname();

      //! Tests parsing a username without a password.
      void TestUsernameOnly();

      //! Tests parsing a password without a username.
      void TestPasswordOnly();

      //! Tests parsing a username and password.
      void TestUsernameAndPassword();

    private:

      CPPUNIT_TEST_SUITE(UriTester);
        CPPUNIT_TEST(TestSchemeOnly);
        CPPUNIT_TEST(TestHostnameOnly);
        CPPUNIT_TEST(TestSchemeAndEmptyHostname);
        CPPUNIT_TEST(TestSchemeAndHostname);
        CPPUNIT_TEST(TestUsernameOnly);
        CPPUNIT_TEST(TestPasswordOnly);
        CPPUNIT_TEST(TestUsernameAndPassword);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
