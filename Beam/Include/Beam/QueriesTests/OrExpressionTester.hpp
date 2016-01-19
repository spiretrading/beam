#ifndef BEAM_OREXPRESSIONTESTER_HPP
#define BEAM_OREXPRESSIONTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class OrExpressionTester
      \brief Tests the OrExpression class.
   */
  class OrExpressionTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests constructing a valid OrExpression.
      void TestConstructor();

      //! Tests constructing OrExpressions with invalid types.
      void TestInvalidConstructor();

      //! Tests making an OrExpression from an empty sequence of
      //! sub-Expressions.
      void TestEmptyMakeOrExpression();

      //! Tests making an OrExpression with a single sub-Expressions.
      void TestSingleMakeOrExpression();

      //! Tests making an OrExpression with a two sub-Expressions.
      void TestTwoSubExpressionMakeOrExpression();

      //! Tests making an OrExpression with multiple sub-Expressions.
      void TestMultipleSubExpressionMakeOrExpression();

    private:
      CPPUNIT_TEST_SUITE(OrExpressionTester);
        CPPUNIT_TEST(TestConstructor);
        CPPUNIT_TEST(TestInvalidConstructor);
        CPPUNIT_TEST(TestEmptyMakeOrExpression);
        CPPUNIT_TEST(TestSingleMakeOrExpression);
        CPPUNIT_TEST(TestTwoSubExpressionMakeOrExpression);
        CPPUNIT_TEST(TestMultipleSubExpressionMakeOrExpression);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
