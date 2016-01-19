#ifndef BEAM_MEMBERACCESSEXPRESSIONTESTER_HPP
#define BEAM_MEMBERACCESSEXPRESSIONTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class MemberAccessExpressionTester
      \brief Tests the MemberAccessExpression class.
   */
  class MemberAccessExpressionTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests constructing a MemberAccessExpression.
      void TestConstructor();

    private:
      CPPUNIT_TEST_SUITE(MemberAccessExpressionTester);
        CPPUNIT_TEST(TestConstructor);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
