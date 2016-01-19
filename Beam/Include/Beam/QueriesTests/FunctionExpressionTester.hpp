#ifndef BEAM_FUNCTIONEXPRESSIONTESTER_HPP
#define BEAM_FUNCTIONEXPRESSIONTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class FunctionExpressionTester
      \brief Tests the FunctionExpression class.
   */
  class FunctionExpressionTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests constructing a FunctionExpression with no parameters.
      void TestEmptyFunction();

      //! Tests constructing a FunctionExpression with one parameter.
      void TestUnaryFunction();

      //! Tests constructing a FunctionExpression with two parameters.
      void TestBinaryFunction();

    private:
      CPPUNIT_TEST_SUITE(FunctionExpressionTester);
        CPPUNIT_TEST(TestEmptyFunction);
        CPPUNIT_TEST(TestUnaryFunction);
        CPPUNIT_TEST(TestBinaryFunction);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
