#ifndef BEAM_FILTERREACTORTESTER_HPP
#define BEAM_FILTERREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class FilterReactorTester
      \brief Tests the FilterReactor class.
   */
  class FilterReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests independent updates.
      void TestIndependentUpdates();

      //! Tests dependent updates.
      void TestDependentUpdates();

    private:
      CPPUNIT_TEST_SUITE(FilterReactorTester);
        CPPUNIT_TEST(TestIndependentUpdates);
        CPPUNIT_TEST(TestDependentUpdates);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
