#ifndef BEAM_DECIMALPARSERTESTER_HPP
#define BEAM_DECIMALPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class DecimalParserTester
      \brief Tests the DecimalParser class.
   */
  class DecimalParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing a positive decimal.
      void TestPositiveDecimal();

      //! Tests parsing a negative decimal.
      void TestNegativeDecimal();

    private:
      CPPUNIT_TEST_SUITE(DecimalParserTester);
        CPPUNIT_TEST(TestPositiveDecimal);
        CPPUNIT_TEST(TestNegativeDecimal);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
