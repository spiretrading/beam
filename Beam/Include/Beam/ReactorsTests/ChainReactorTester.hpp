#ifndef BEAM_CHAINREACTORTESTER_HPP
#define BEAM_CHAINREACTORTESTER_HPP
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

      //! Tests chaining two ranges together.
      void TestRange();

    private:
      CPPUNIT_TEST_SUITE(ChainReactorTester);
        CPPUNIT_TEST(TestRange);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
