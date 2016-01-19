#ifndef BEAM_REDUCEEXPRESSIONTESTER_HPP
#define BEAM_REDUCEEXPRESSIONTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ReduceExpressionTester
      \brief Tests the ReduceExpression class.
   */
  class ReduceExpressionTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests constructing a ReduceExpression with compatible types.
      void TestCompatibleTypeConstructor();

      //! Tests constructing a ReduceExpression with incompatible types.
      void TestIncompatibleTypeConstructor();

    private:
      CPPUNIT_TEST_SUITE(ReduceExpressionTester);
        CPPUNIT_TEST(TestCompatibleTypeConstructor);
        CPPUNIT_TEST(TestIncompatibleTypeConstructor);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
