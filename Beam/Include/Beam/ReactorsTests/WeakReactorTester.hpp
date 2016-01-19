#ifndef BEAM_WEAKREACTORTESTER_HPP
#define BEAM_WEAKREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class WeakReactorTester
      \brief Tests the WeakReactor class.
   */
  class WeakReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests updating a WeakReactor.
      void TestUpdate();

    private:
      CPPUNIT_TEST_SUITE(WeakReactorTester);
        CPPUNIT_TEST(TestUpdate);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
