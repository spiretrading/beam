#ifndef BEAM_REACTOR_TASK_TESTER_HPP
#define BEAM_REACTOR_TASK_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class ReactorTaskTester
      \brief Tests the ReactorTask class.
   */
  class ReactorTaskTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a ReactorTask by updating a Reactor.
      void TestUpdates();

    private:
      CPPUNIT_TEST_SUITE(ReactorTaskTester);
        CPPUNIT_TEST(TestUpdates);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
