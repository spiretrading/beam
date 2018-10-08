#ifndef BEAM_SQL_DATA_STORE_TESTER_HPP
#define BEAM_SQL_DATA_STORE_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam::Queries::Tests {

  /** Tests the SqlDataStore class. */
  class SqlDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests storing and loading a value.
      void TestStoreAndLoad();

      //! Tests using an index embedded into the value.
      void TestEmbeddedIndex();

    private:
      CPPUNIT_TEST_SUITE(SqlDataStoreTester);
        CPPUNIT_TEST(TestStoreAndLoad);
        CPPUNIT_TEST(TestEmbeddedIndex);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
