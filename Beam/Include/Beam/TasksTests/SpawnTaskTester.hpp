#ifndef BEAM_SPAWNTASKTESTER_HPP
#define BEAM_SPAWNTASKTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class SpawnTaskTester
      \brief Tests the SpawnTask class.
   */
  class SpawnTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests triggering a SpawnTask.
      void TestTrigger();

    private:
      CPPUNIT_TEST_SUITE(SpawnTaskTester);
        CPPUNIT_TEST(TestTrigger);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
