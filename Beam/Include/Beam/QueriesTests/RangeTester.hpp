#ifndef BEAM_RANGETESTER_HPP
#define BEAM_RANGETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class RangeTester
      \brief Tests the Range class.
   */
  class RangeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests constructing empty Ranges.
      void TestEmptyRangeConstructors();

      //! Tests constructing total Ranges.
      void TestTotalRangeConstructors();

      //! Tests constructing out of order Ranges.
      void TestOutOfOrderConstructors();

      //! Tests constructing Ranges defined by Sequences.
      void TestSequenceConstructors();

      //! Tests constructing Ranges defined by dates.
      void TestDateConstructors();

      //! Tests constructing Ranges defined by both dates and Sequences.
      void TestMixedConstructors();

      //! Tests the Empty Range value.
      void TestEmptyRangeValue();

      //! Tests the Total Range value.
      void TestTotalRangeValue();

      //! Tests the RealTime Range value.
      void TestRealTimeRangeValue();

      //! Tests the equals operator.
      void TestEqualsOperator();

      //! Tests the not equals operator.
      void TestNotEqualsOperator();

    private:
      CPPUNIT_TEST_SUITE(RangeTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestEmptyRangeConstructors);
        CPPUNIT_TEST(TestTotalRangeConstructors);
        CPPUNIT_TEST(TestOutOfOrderConstructors);
        CPPUNIT_TEST(TestSequenceConstructors);
        CPPUNIT_TEST(TestDateConstructors);
        CPPUNIT_TEST(TestMixedConstructors);
        CPPUNIT_TEST(TestEmptyRangeValue);
        CPPUNIT_TEST(TestTotalRangeValue);
        CPPUNIT_TEST(TestRealTimeRangeValue);
        CPPUNIT_TEST(TestEqualsOperator);
        CPPUNIT_TEST(TestNotEqualsOperator);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
