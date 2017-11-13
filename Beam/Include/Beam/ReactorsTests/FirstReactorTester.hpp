#ifndef BEAM_FIRST_REACTOR_TESTER_HPP
#define BEAM_FIRST_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class FirstReactorTester
      \brief Tests the static Reactor.
   */
  class FirstReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests that the static reactor only evaluates to a single value and
      //! then comes to completion.
      void TestSingleValue();

    private:
      CPPUNIT_TEST_SUITE(FirstReactorTester);
        CPPUNIT_TEST(TestSingleValue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
