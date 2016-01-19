#ifndef BEAM_REDUCEEVALUATORNODETESTER_HPP
#define BEAM_REDUCEEVALUATORNODETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ReduceEvaluatorNodeTester
      \brief Tests the ReduceEvaluatorNode class.
   */
  class ReduceEvaluatorNodeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests reducing a sum.
      void TestReduceSum();

    private:
      CPPUNIT_TEST_SUITE(ReduceEvaluatorNodeTester);
        CPPUNIT_TEST(TestReduceSum);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
