#ifndef BEAM_EVALUATORTESTER_HPP
#define BEAM_EVALUATORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class EvaluatorTester
      \brief Tests the Evaluator class.
   */
  class EvaluatorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests evaluating a ConstantExpression.
      void TestConstantExpression();

      //! Tests evaluating an addition Expression.
      void TestAdditionExpression();

      //! Tests evaluating a ParameterExpression.
      void TestParameterExpression();

      //! Tests evaluating a ReduceExpression.
      void TestReduceExpression();

      //! Tests evaluating a ParameterExpression whose index is out of range.
      void TestOutOfRangeParameterExpression();

      //! Tests ParameterExpressions with mismatched types.
      void TestMismatchedTypeParameterExpressions();

      //! Tests gaps in ParameterExpression indicies.
      void TestParameterExpressionGap();

    private:
      CPPUNIT_TEST_SUITE(EvaluatorTester);
        CPPUNIT_TEST(TestConstantExpression);
        CPPUNIT_TEST(TestAdditionExpression);
        CPPUNIT_TEST(TestParameterExpression);
        CPPUNIT_TEST(TestReduceExpression);
        CPPUNIT_TEST(TestOutOfRangeParameterExpression);
        CPPUNIT_TEST(TestMismatchedTypeParameterExpressions);
        CPPUNIT_TEST(TestParameterExpressionGap);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
