#ifndef BEAM_FUNCTION_REACTOR_TESTER_HPP
#define BEAM_FUNCTION_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class FunctionReactorTester
      \brief Tests the FunctionReactor class.
   */
  class FunctionReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a function with no parameters.
      void TestNoParameters();

      //! Tests a function with no parameters that throws.
      void TestNoParametersThrow();

      //! Tests a function with no parameters that produces no output.
      void TestNoParametersEmpty();

      //! Tests a function with one constant parameter.
      void TestOneConstantParameter();

      //! Tests a function with one parameter that never produces an evaluation.
      void TestOneParameterNoEval();

      //! Tests a function with one parameter that produces a single evaluation.
      void TestOneParameterWithSingleEval();

      //! Tests a function with one parameter that produces multiple
      //! evaluations.
      void TestOneParameterWithMultipleEvals();

      //! Tests a function with one parameter that filters some evaluations.
      void TestOneParameterWithFilter();

      //! Tests a function of two parameters that are both constants.
      void TestTwoConstantParameters();

      //! Tests a void function.
      void TestVoidFunction();

    private:
      CPPUNIT_TEST_SUITE(FunctionReactorTester);
        CPPUNIT_TEST(TestNoParameters);
        CPPUNIT_TEST(TestNoParametersThrow);
        CPPUNIT_TEST(TestNoParametersEmpty);
        CPPUNIT_TEST(TestOneConstantParameter);
        CPPUNIT_TEST(TestOneParameterNoEval);
        CPPUNIT_TEST(TestOneParameterWithSingleEval);
        CPPUNIT_TEST(TestOneParameterWithMultipleEvals);
        CPPUNIT_TEST(TestOneParameterWithFilter);
        CPPUNIT_TEST(TestTwoConstantParameters);
        CPPUNIT_TEST(TestVoidFunction);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
