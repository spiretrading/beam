#ifndef BEAM_SESSIONCACHEDDATASTORETESTER_HPP
#define BEAM_SESSIONCACHEDDATASTORETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SessionCachedDataStoreTester
      \brief Tests the SessionCachedDataStore class.
   */
  class SessionCachedDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests storing and loading a value.
      void TestStoreAndLoad();

      //! Tests cache coherence when new items are added to the data store.
      void TestForwardCoherence();

    private:
      CPPUNIT_TEST_SUITE(SessionCachedDataStoreTester);
        CPPUNIT_TEST(TestStoreAndLoad);
        CPPUNIT_TEST(TestForwardCoherence);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
