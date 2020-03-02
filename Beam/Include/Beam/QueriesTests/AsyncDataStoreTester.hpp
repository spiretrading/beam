#ifndef BEAM_ASYNC_DATA_STORE_TESTER_HPP
#define BEAM_ASYNC_DATA_STORE_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam::Queries::Tests {

  /** Tests the AsyncDataStore class. */
  class AsyncDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

    private:
      CPPUNIT_TEST_SUITE(AsyncDataStoreTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
