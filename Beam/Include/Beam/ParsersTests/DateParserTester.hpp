#ifndef BEAM_DATEPARSERTESTER_HPP
#define BEAM_DATEPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class DateParserTester
      \brief Tests the DateParser class.
   */
  class DateParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests reading a date.
      void TestReadDate();

    private:
      CPPUNIT_TEST_SUITE(DateParserTester);
        CPPUNIT_TEST(TestReadDate);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
