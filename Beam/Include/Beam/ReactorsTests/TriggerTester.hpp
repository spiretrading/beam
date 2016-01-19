#ifndef BEAM_TRIGGERTESTER_HPP
#define BEAM_TRIGGERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class TriggerTester
      \brief Tests the Trigger class.
   */
  class TriggerTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests triggering an action immediately after a ReactorMonitor open's.
      void TestOpenTrigger();

    private:
      CPPUNIT_TEST_SUITE(TriggerTester);
        CPPUNIT_TEST(TestOpenTrigger);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
