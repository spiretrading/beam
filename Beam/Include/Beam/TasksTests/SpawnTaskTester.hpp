#ifndef BEAM_SPAWN_TASK_TESTER_HPP
#define BEAM_SPAWN_TASK_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class SpawnTaskTester
      \brief Tests the SpawnTask class.
   */
  class SpawnTaskTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a SpawnTask where the trigger immediately completes.
      void TestImmediateCompletion();

    private:
      CPPUNIT_TEST_SUITE(SpawnTaskTester);
        CPPUNIT_TEST(TestImmediateCompletion);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
