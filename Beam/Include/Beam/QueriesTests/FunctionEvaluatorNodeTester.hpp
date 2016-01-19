#ifndef BEAM_FUNCTIONEVALUATORNODETESTER_HPP
#define BEAM_FUNCTIONEVALUATORNODETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class FunctionEvaluatorNodeTester
      \brief Tests the FunctionEvaluatorNode class.
   */
  class FunctionEvaluatorNodeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a function with no parameters.
      void TestEmptyFunction();

      //! Tests a function with one parameter.
      void TestUnaryFunction();

      //! Tests a function with two parameters.
      void TestBinaryFunction();

    private:
      CPPUNIT_TEST_SUITE(FunctionEvaluatorNodeTester);
        CPPUNIT_TEST(TestEmptyFunction);
        CPPUNIT_TEST(TestUnaryFunction);
        CPPUNIT_TEST(TestBinaryFunction);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
