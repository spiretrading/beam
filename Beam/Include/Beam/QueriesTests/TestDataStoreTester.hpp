#ifndef BEAM_TEST_DATA_STORE_TESTER_HPP
#define BEAM_TEST_DATA_STORE_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam::Queries::Tests {

  /** Tests the TestDataStore class. */
  class TestDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      /** Tests an Open operation with exception. */
      void TestOpenException();

      /** Tests a Store operation without exception. */
      void TestStore();

      /** Tests a Store operation with exception. */
      void TestStoreException();

      /** Tests a Load operation without exception. */
      void TestLoad();

      /** Tests a Load operation with exception. */
      void TestLoadException();

    private:
      CPPUNIT_TEST_SUITE(TestDataStoreTester);
        CPPUNIT_TEST(TestOpenException);
        CPPUNIT_TEST(TestStore);
        CPPUNIT_TEST(TestStoreException);
        CPPUNIT_TEST(TestLoad);
        CPPUNIT_TEST(TestLoadException);
        CPPUNIT_TEST(TestStore);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
