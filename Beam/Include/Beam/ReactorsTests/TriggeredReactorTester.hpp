#ifndef BEAM_TRIGGEREDREACTORTESTER_HPP
#define BEAM_TRIGGEREDREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class TriggeredReactorTester
      \brief Tests the TriggeredReactor class.
   */
  class TriggeredReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests updating a TriggeredReactor.
      void TestUpdate();

    private:
      CPPUNIT_TEST_SUITE(TriggeredReactorTester);
        CPPUNIT_TEST(TestUpdate);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
