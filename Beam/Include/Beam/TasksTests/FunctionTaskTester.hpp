#ifndef BEAM_FUNCTION_TASK_TESTER_HPP
#define BEAM_FUNCTION_TASK_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class FunctionTaskTester
      \brief Tests the function Task.
   */
  class FunctionTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests executing a function with no parameters.
      void TestFunctionZeroParameters();

      //! Tests executing a function with one parameter.
      void TestFunctionOneParameter();

    private:
      CPPUNIT_TEST_SUITE(FunctionTaskTester);
        CPPUNIT_TEST(TestFunctionZeroParameters);
        CPPUNIT_TEST(TestFunctionOneParameter);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
