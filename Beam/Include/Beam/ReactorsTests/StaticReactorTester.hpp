#ifndef BEAM_STATIC_REACTOR_TESTER_HPP
#define BEAM_STATIC_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class StaticReactorTester
      \brief Tests the static Reactor.
   */
  class StaticReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests that the static reactor only evaluates to a single value and
      //! then comes to completion.
      void TestSingleValue();

    private:
      CPPUNIT_TEST_SUITE(StaticReactorTester);
        CPPUNIT_TEST(TestSingleValue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
