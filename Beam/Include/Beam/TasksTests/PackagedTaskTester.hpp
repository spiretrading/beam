#ifndef BEAM_PACKAGED_TASK_TESTER_HPP
#define BEAM_PACKAGED_TASK_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class PackagedTaskTester
      \brief Tests the PackagedTask class.
   */
  class PackagedTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests executing a function.
      void TestExecuteFunction();

    private:
      CPPUNIT_TEST_SUITE(PackagedTaskTester);
        CPPUNIT_TEST(TestExecuteFunction);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
