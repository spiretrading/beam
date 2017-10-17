#ifndef BEAM_NON_REPEATING_REACTOR_TESTER_HPP
#define BEAM_NON_REPEATING_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class NonRepeatingReactorTester
      \brief Tests the non-repeating Reactor.
   */
  class NonRepeatingReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Produce the value 1.
      //! Expect the value 1 to be evaluated.
      //! Produce the value 1.
      //! Expect no change.
      //! Produce the value 2.
      //! Expect the value 2 to be evaluated.
      void TestAAB();

      //! Produce the value 1.
      //! Expect the value 1 to be evaluated.
      //! Produce the value 2.
      //! Expect the value 2 to be evaluated.
      //! Produce the value 1.
      //! Expect the value 1 to be evaluated.
      void TestABA();

      //! Produce the value 1.
      //! Expect the value 1 to be evaluated.
      //! Produce the value 2.
      //! Expect the value 2 to be evaluated.
      //! Produce the value 2.
      //! Expect no change.
      void TestBAA();

    private:
      CPPUNIT_TEST_SUITE(NonRepeatingReactorTester);
        CPPUNIT_TEST(TestAAB);
        CPPUNIT_TEST(TestABA);
        CPPUNIT_TEST(TestBAA);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
