#ifndef BEAM_ASYNC_DATA_STORE_TESTER_HPP
#define BEAM_ASYNC_DATA_STORE_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam::Queries::Tests {

  /** Tests the AsyncDataStore class. */
  class AsyncDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests storing and loading a value.
      void TestStoreAndLoad();

      //! Tests loading a series of values that span from the latests to
      //! the earliest.
      void TestHeadSpanningLoad();

      //! Tests loading a series of values that span from the earliest to
      //! the latest.
      void TestTailSpanningLoad();

    private:
      CPPUNIT_TEST_SUITE(AsyncDataStoreTester);
        CPPUNIT_TEST(TestStoreAndLoad);
        CPPUNIT_TEST(TestHeadSpanningLoad);
        CPPUNIT_TEST(TestTailSpanningLoad);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
