#ifndef BEAM_RANGEDQUERYTESTER_HPP
#define BEAM_RANGEDQUERYTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class RangedQueryTester
      \brief Tests the RangedQuery class.
   */
  class RangedQueryTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests the constructor specifying a Range.
      void TestRangeConstructor();

      //! Tests setting a Range.
      void TestSetRange();

    private:
      CPPUNIT_TEST_SUITE(RangedQueryTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestRangeConstructor);
        CPPUNIT_TEST(TestSetRange);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
