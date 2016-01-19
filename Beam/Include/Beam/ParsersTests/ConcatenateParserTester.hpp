#ifndef BEAM_CONCATENATEPARSERTESTER_HPP
#define BEAM_CONCATENATEPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class ConcatenateParserTester
      \brief Tests the ConcatenateParser class.
   */
  class ConcatenateParserTester : public CPPUNIT_NS::TestFixture {
    public:

      void TestVoidParsers();

      void TestLeftVoidParsers();

      void TestRightVoidParsers();

      void TestNoVoidParsers();

    private:
      CPPUNIT_TEST_SUITE(ConcatenateParserTester);
        CPPUNIT_TEST(TestVoidParsers);
        CPPUNIT_TEST(TestLeftVoidParsers);
        CPPUNIT_TEST(TestRightVoidParsers);
        CPPUNIT_TEST(TestNoVoidParsers);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
