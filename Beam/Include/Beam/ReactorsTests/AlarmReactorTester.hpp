#ifndef BEAM_ALARMREACTORTESTER_HPP
#define BEAM_ALARMREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class AlarmReactorTester
      \brief Tests an alarm Reactor.
   */
  class AlarmReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests expiring the Reactor in the future and then the past.
      void TestFutureThenPastExpiry();

      //! Tests expiring the Reactor in the past and then future.
      void TestPastThenFutureExpiry();

    private:
      CPPUNIT_TEST_SUITE(AlarmReactorTester);
        CPPUNIT_TEST(TestFutureThenPastExpiry);
        CPPUNIT_TEST(TestPastThenFutureExpiry);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
