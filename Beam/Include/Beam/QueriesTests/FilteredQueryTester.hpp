#ifndef BEAM_FILTEREDQUERYTESTER_HPP
#define BEAM_FILTEREDQUERYTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class FilteredQueryTester
      \brief Tests the FilteredQuery class.
   */
  class FilteredQueryTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests the constructor specifying a filter.
      void TestFilterConstructor();

      //! Tests setting a filter.
      void TestSetFilter();

    private:
      CPPUNIT_TEST_SUITE(FilteredQueryTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestFilterConstructor);
        CPPUNIT_TEST(TestSetFilter);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
