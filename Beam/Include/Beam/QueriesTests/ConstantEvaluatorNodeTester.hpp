#ifndef BEAM_CONSTANTEVALUATORNODETESTER_HPP
#define BEAM_CONSTANTEVALUATORNODETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ConstantEvaluatorNodeTester
      \brief Tests the ConstantEvaluatorNode class.
   */
  class ConstantEvaluatorNodeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests making a ConstantEvaluatorNodeTester.
      void TestMakeConstantEvaluatorNode();

      //! Tests an int ConstantEvaluatorNode.
      void TestInt();

      //! Tests a decimal ConstantEvaluatorNode.
      void TestDecimal();

      //! Tests a string ConstantEvaluatorNode.
      void TestString();

    private:
      CPPUNIT_TEST_SUITE(ConstantEvaluatorNodeTester);
        CPPUNIT_TEST(TestMakeConstantEvaluatorNode);
        CPPUNIT_TEST(TestInt);
        CPPUNIT_TEST(TestDecimal);
        CPPUNIT_TEST(TestString);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
