#ifndef BEAM_CONSTANTEXPRESSIONTESTER_HPP
#define BEAM_CONSTANTEXPRESSIONTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ConstantExpressionTester
      \brief Tests the ConstantExpression class.
   */
  class ConstantExpressionTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests making a ConstantExpression.
      void TestMakeConstantExpression();

      //! Tests an int ConstantExpression.
      void TestInt();

      //! Tests a decimal ConstantExpression.
      void TestDecimal();

      //! Tests a string ConstantExpression.
      void TestString();

    private:
      CPPUNIT_TEST_SUITE(ConstantExpressionTester);
        CPPUNIT_TEST(TestMakeConstantExpression);
        CPPUNIT_TEST(TestInt);
        CPPUNIT_TEST(TestDecimal);
        CPPUNIT_TEST(TestString);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
