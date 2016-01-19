#ifndef BEAM_DATETIMEPARSERTESTER_HPP
#define BEAM_DATETIMEPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class DateTimeParserTester
      \brief Tests the DateTimeParser class.
   */
  class DateTimeParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests reading a date/time.
      void TestReadDateTime();

    private:
      CPPUNIT_TEST_SUITE(DateTimeParserTester);
        CPPUNIT_TEST(TestReadDateTime);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
