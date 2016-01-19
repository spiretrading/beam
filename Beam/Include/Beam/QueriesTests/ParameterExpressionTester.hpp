#ifndef BEAM_PARAMETEREXPRESSIONTESTER_HPP
#define BEAM_PARAMETEREXPRESSIONTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ParameterExpressionTester
      \brief Tests the ParameterExpression class.
   */
  class ParameterExpressionTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests constructing a ParameterExpression.
      void TestConstructor();

    private:
      CPPUNIT_TEST_SUITE(ParameterExpressionTester);
        CPPUNIT_TEST(TestConstructor);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
