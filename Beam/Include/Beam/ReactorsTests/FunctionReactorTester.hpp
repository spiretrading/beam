#ifndef BEAM_FUNCTIONREACTORTESTER_HPP
#define BEAM_FUNCTIONREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*! \class FunctionReactorTester
      \brief Tests the FunctionReactor class.
   */
  class FunctionReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests no parameters.
      void TestNoParameters();

      //! Tests a single parameter.
      void TestSingleParameter();

      //! Tests two parameters.
      void TestTwoParameters();

      //! Tests a range of values.
      void TestRange();

    private:
      CPPUNIT_TEST_SUITE(FunctionReactorTester);
        CPPUNIT_TEST(TestNoParameters);
        CPPUNIT_TEST(TestSingleParameter);
        CPPUNIT_TEST(TestTwoParameters);
        CPPUNIT_TEST(TestRange);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
