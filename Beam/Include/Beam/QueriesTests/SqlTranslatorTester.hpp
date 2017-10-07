#ifndef BEAM_SQLTRANSLATORTESTER_HPP
#define BEAM_SQLTRANSLATORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SqlTranslatorTester
      \brief Tests the SqlTranslator class.
   */
  class SqlTranslatorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests an OR expression.
      void TestOrExpression();

    private:
      CPPUNIT_TEST_SUITE(SqlTranslatorTester);
        CPPUNIT_TEST(TestOrExpression);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
