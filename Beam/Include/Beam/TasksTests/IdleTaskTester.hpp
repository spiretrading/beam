#ifndef BEAM_IDLETASKTESTER_HPP
#define BEAM_IDLETASKTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class IdleTaskTester
      \brief Tests the IdleTask class.
   */
  class IdleTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests executing and canceling a IdleTask.
      void TestExecuteAndCancel();

    private:
      CPPUNIT_TEST_SUITE(IdleTaskTester);
        CPPUNIT_TEST(TestExecuteAndCancel);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
