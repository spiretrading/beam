#ifndef BEAM_CACHEDDATASTORETESTER_HPP
#define BEAM_CACHEDDATASTORETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class CachedDataStoreTester
      \brief Tests the CachedDataStore class.
   */
  class CachedDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests storing and loading a value.
      void TestStoreAndLoad();

      //! Tests cache coherence when new items are added to the data store.
      void TestForwardCoherence();

      //! Tests cache coherence when old items are added to the data store.
      void TestBackwardCoherence();

    private:
      CPPUNIT_TEST_SUITE(CachedDataStoreTester);
        CPPUNIT_TEST(TestStoreAndLoad);
        CPPUNIT_TEST(TestForwardCoherence);
        CPPUNIT_TEST(TestBackwardCoherence);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
