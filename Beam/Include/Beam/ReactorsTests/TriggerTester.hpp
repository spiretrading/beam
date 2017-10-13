#ifndef BEAM_TRIGGER_TESTER_HPP
#define BEAM_TRIGGER_TESTER_HPP
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

      //! Tests signaling an update.
      void TestSignalUpdate();

    private:
      CPPUNIT_TEST_SUITE(TriggerTester);
        CPPUNIT_TEST(TestSignalUpdate);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
