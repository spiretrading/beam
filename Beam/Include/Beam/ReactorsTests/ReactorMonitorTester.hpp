#ifndef BEAM_REACTOR_MONITOR_TESTER_HPP
#define BEAM_REACTOR_MONITOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class ReactorMonitorTester
      \brief Tests the ReactorMonitor class.
   */
  class ReactorMonitorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests adding a single Reactor before the Open method is called.
      void TestAddSingleReactorBeforeOpen();

      //! Tests adding a single Reactor after the Open method is called.
      void TestAddSingleReactorAfterOpen();

    private:
      CPPUNIT_TEST_SUITE(ReactorMonitorTester);
        CPPUNIT_TEST(TestAddSingleReactorBeforeOpen);
        CPPUNIT_TEST(TestAddSingleReactorAfterOpen);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
