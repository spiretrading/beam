#ifndef BEAM_UNTILTASKTESTER_HPP
#define BEAM_UNTILTASKTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class UntilTaskTester
      \brief Tests the UntilTask class.
   */
  class UntilTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests executing an UntilTask and then triggering the condition.
      void TestExecuteThenTrigger();

      //! Tests triggering a condition and then executing an UntilTask.
      void TestTriggerThenExecute();

    private:
      CPPUNIT_TEST_SUITE(UntilTaskTester);
        CPPUNIT_TEST(TestExecuteThenTrigger);
        CPPUNIT_TEST(TestTriggerThenExecute);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
