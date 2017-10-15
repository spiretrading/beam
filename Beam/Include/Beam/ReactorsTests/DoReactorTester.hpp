#ifndef BEAM_DO_REACTOR_TESTER_HPP
#define BEAM_DO_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class DoReactorTester
      \brief Tests the Do Reactor.
   */
  class DoReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests that the value passed to the Do Reactor is the same as
      //! the one produced by its parameter.
      void TestPassThrough();

    private:
      CPPUNIT_TEST_SUITE(DoReactorTester);
        CPPUNIT_TEST(TestPassThrough);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
