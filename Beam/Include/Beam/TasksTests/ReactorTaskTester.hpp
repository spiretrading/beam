#ifndef BEAM_REACTORTASKTESTER_HPP
#define BEAM_REACTORTASKTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/TasksTests/TasksTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class ReactorTaskTester
      \brief Tests the ReactorTask class.
   */
  class ReactorTaskTester  : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a ReactorTask with a single property.
      void TestOneProperty();

    private:
      CPPUNIT_TEST_SUITE(ReactorTaskTester);
        CPPUNIT_TEST(TestOneProperty);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
