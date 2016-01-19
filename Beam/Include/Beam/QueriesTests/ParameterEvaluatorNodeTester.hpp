#ifndef BEAM_PARAMETEREVALUATORNODETESTER_HPP
#define BEAM_PARAMETEREVALUATORNODETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ParameterEvaluatorNodeTester
      \brief Tests the ParameterEvaluatorNode class.
   */
  class ParameterEvaluatorNodeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests an int parameter.
      void TestInt();

    private:
      CPPUNIT_TEST_SUITE(ParameterEvaluatorNodeTester);
        CPPUNIT_TEST(TestInt);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
