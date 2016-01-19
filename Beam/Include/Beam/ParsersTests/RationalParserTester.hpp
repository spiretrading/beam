#ifndef BEAM_RATIONALPARSERTESTER_HPP
#define BEAM_RATIONALPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class RationalParserTester
      \brief Tests the RationalParser class.
   */
  class RationalParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing whole numbers.
      void TestWholeNumbers();

      //! Tests parsing a value between 0 and 1.
      void TestWithinZeroAndOne();

    private:
      CPPUNIT_TEST_SUITE(RationalParserTester);
        CPPUNIT_TEST(TestWholeNumbers);
        CPPUNIT_TEST(TestWithinZeroAndOne);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
