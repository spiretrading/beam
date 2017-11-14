#ifndef BEAM_CHAINED_TASK_TESTER_HPP
#define BEAM_CHAINED_TASK_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class ChainedTaskTester
      \brief Tests the ChainedTask class.
   */
  class ChainedTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests executing an empty ChainedTask.
      void TestEmpty();

      //! Tests executing a ChainedTask with a single Task.
      void TestSingleTask();

      //! Tests executing a ChainedTask with two Tasks.
      void TestDoubleTask();

      //! Tests executing a ChainedTask with two Tasks where the first fails.
      void TestDoubleTaskWithFailure();

    private:
      CPPUNIT_TEST_SUITE(ChainedTaskTester);
        CPPUNIT_TEST(TestEmpty);
        CPPUNIT_TEST(TestSingleTask);
        CPPUNIT_TEST(TestDoubleTask);
        CPPUNIT_TEST(TestDoubleTaskWithFailure);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
