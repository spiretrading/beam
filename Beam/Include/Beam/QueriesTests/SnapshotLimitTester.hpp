#ifndef BEAM_SNAPSHOTLIMITTESTER_HPP
#define BEAM_SNAPSHOTLIMITTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SnapshotLimitTester
      \brief Tests the SnapshotLimit class.
   */
  class SnapshotLimitTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests constructing a SnapshotLimit with various limits.
      void TestConstructor();

      //! Tests the None SnapshotLimit.
      void TestNoneSnapshotLimit();

      //! Tests the Unlimited SnapshotLimit.
      void TestUnlimitedSnapshotLimit();

      //! Tests the equals operator.
      void TestEqualsOperator();

      //! Tests the not equals operator.
      void TestNotEqualsOperator();

    private:
      CPPUNIT_TEST_SUITE(SnapshotLimitTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestConstructor);
        CPPUNIT_TEST(TestNoneSnapshotLimit);
        CPPUNIT_TEST(TestUnlimitedSnapshotLimit);
        CPPUNIT_TEST(TestEqualsOperator);
        CPPUNIT_TEST(TestNotEqualsOperator);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
