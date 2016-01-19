#ifndef BEAM_MEMBERACCESSEVALUATOENODETESTER_HPP
#define BEAM_MEMBERACCESSEVALUATOENODETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class MemberAccessEvaluatorNodeTester
      \brief Tests the MemberAccessEvaluatorNode class.
   */
  class MemberAccessEvaluatorNodeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests constructing a MemberAccessEvaluatorNode.
      void TestConstructor();

    private:
      CPPUNIT_TEST_SUITE(MemberAccessEvaluatorNodeTester);
        CPPUNIT_TEST(TestConstructor);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
