#ifndef BEAM_CHAIN_REACTOR_TESTER_HPP
#define BEAM_CHAIN_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class ChainReactorTester
      \brief Tests the ChainReactor class.
   */
  class ChainReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests chaining two constants together.
      void TestConstantChain();

      //! Tests chaining a constant with a NoneReactor.
      void TestSingleValue();

    private:
      CPPUNIT_TEST_SUITE(ChainReactorTester);
        CPPUNIT_TEST(TestConstantChain);
        CPPUNIT_TEST(TestSingleValue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
