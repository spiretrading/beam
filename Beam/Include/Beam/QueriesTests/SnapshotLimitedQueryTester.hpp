#ifndef BEAM_SNAPSHOTLIMITEDQUERYTESTER_HPP
#define BEAM_SNAPSHOTLIMITEDQUERYTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SnapshotLimitedQueryTester
      \brief Tests the SnapshotLimitedQuery class.
   */
  class SnapshotLimitedQueryTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests the constructor specifying a SnapshotLimit.
      void TestSnapshotLimitConstructor();

      //! Tests setting a SnapshotLimit.
      void TestSetSnapshotLimit();

    private:
      CPPUNIT_TEST_SUITE(SnapshotLimitedQueryTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestSnapshotLimitConstructor);
        CPPUNIT_TEST(TestSetSnapshotLimit);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
