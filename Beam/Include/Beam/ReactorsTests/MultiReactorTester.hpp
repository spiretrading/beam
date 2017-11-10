#ifndef BEAM_MULTI_REACTOR_TESTER_HPP
#define BEAM_MULTI_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class MultiReactorTester
      \brief Tests the MultiReactor class.
   */
  class MultiReactorTester : public CPPUNIT_NS::TestFixture {
    public:
    private:
      CPPUNIT_TEST_SUITE(MultiReactorTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
