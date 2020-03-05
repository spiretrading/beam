#ifndef BEAM_TEST_DATA_STORE_TESTER_HPP
#define BEAM_TEST_DATA_STORE_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam::Queries::Tests {

  /** Tests the TestDataStore class. */
  class TestDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      /** Tests a Store operation. */
      void TestStore();

    private:
      CPPUNIT_TEST_SUITE(TestDataStoreTester);
        CPPUNIT_TEST(TestStore);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
