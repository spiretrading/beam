#ifndef BEAM_NONE_REACTOR_TESTER_HPP
#define BEAM_NONE_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class NoneReactorTester
      \brief Tests the NoneReactor class.
   */
  class NoneReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests an int NoneReactor.
      void TestInt();

    private:
      CPPUNIT_TEST_SUITE(NoneReactorTester);
        CPPUNIT_TEST(TestInt);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
