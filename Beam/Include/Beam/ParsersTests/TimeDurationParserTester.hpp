#ifndef BEAM_TIMEDURATIONPARSERTESTER_HPP
#define BEAM_TIMEDURATIONPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class TimeDurationParserTester
      \brief Tests the TimeDurationParser class.
   */
  class TimeDurationParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests reading a time_duration.
      void TestReadTimeDuration();

    private:
      CPPUNIT_TEST_SUITE(TimeDurationParserTester);
        CPPUNIT_TEST(TestReadTimeDuration);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
