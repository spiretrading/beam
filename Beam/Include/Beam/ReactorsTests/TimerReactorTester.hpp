#ifndef BEAM_TIMER_REACTOR_TESTER_HPP
#define BEAM_TIMER_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class TimerReactorTester
      \brief Tests the Timer Reactor.
   */
  class TimerReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests expiring the timer.
      void TestExpiry();

    private:
      CPPUNIT_TEST_SUITE(TimerReactorTester);
        CPPUNIT_TEST(TestExpiry);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
