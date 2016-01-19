#ifndef BEAM_INTEGRALPARSERTESTER_HPP
#define BEAM_INTEGRALPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class IntegralParserTester
      \brief Tests the IntegralParser class.
   */
  class IntegralParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing a positive int.
      void TestPositiveInt();

      //! Tests parsing a negative int.
      void TestNegativeInt();

    private:
      CPPUNIT_TEST_SUITE(IntegralParserTester);
        CPPUNIT_TEST(TestPositiveInt);
        CPPUNIT_TEST(TestNegativeInt);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
