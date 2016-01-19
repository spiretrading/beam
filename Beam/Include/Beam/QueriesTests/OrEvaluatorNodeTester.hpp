#ifndef BEAM_OREVALUATORNODETESTER_HPP
#define BEAM_OREVALUATORNODETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class OrEvaluatorNodeTester
      \brief Tests the OrEvaluatorNode class.
   */
  class OrEvaluatorNodeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests constructing an OrEvaluatorNode.
      void TestConstructor();

    private:
      CPPUNIT_TEST_SUITE(OrEvaluatorNodeTester);
        CPPUNIT_TEST(TestConstructor);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
