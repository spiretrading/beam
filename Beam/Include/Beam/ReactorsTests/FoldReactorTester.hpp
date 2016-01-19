#ifndef BEAM_FOLDREACTORTESTER_HPP
#define BEAM_FOLDREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class FoldReactorTester
      \brief Tests the FoldReactor class.
   */
  class FoldReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests adding up a series of ints.
      void TestSum();

    private:
      CPPUNIT_TEST_SUITE(FoldReactorTester);
        CPPUNIT_TEST(TestSum);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
