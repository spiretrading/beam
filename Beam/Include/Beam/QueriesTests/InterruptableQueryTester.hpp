#ifndef BEAM_INTERRUPTABLEQUERYTESTER_HPP
#define BEAM_INTERRUPTABLEQUERYTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class InterruptableQueryTester
      \brief Tests the InterruptableQuery class.
   */
  class InterruptableQueryTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests the constructor specifying an InterruptionPolicy.
      void TestInterruptionPolicyConstructor();

      //! Tests setting an InterruptionPolicy.
      void TestSetInterruptionPolicy();

    private:
      CPPUNIT_TEST_SUITE(InterruptableQueryTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestInterruptionPolicyConstructor);
        CPPUNIT_TEST(TestSetInterruptionPolicy);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
