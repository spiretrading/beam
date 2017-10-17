#ifndef BEAM_WHENTASKTESTER_HPP
#define BEAM_WHENTASKTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class WhenTaskTester
      \brief Tests the WhenTask class.
   */
  class WhenTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests executing a WhenTask where the trigger never signals the
      //! condition.
      void TestNeverTrigger();

      //! Tests executing a WhenTask and then triggering the condition.
      void TestExecuteThenTrigger();

      //! Tests triggering a condition and then executing a WhenTask.
      void TestTriggerThenExecute();

    private:
      CPPUNIT_TEST_SUITE(WhenTaskTester);
        CPPUNIT_TEST(TestNeverTrigger);
        CPPUNIT_TEST(TestExecuteThenTrigger);
        CPPUNIT_TEST(TestTriggerThenExecute);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
