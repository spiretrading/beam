#ifndef BEAM_SEQUENCETESTER_HPP
#define BEAM_SEQUENCETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SequenceTester
      \brief Tests the Sequence class.
   */
  class SequenceTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests the constructor taking only the ordinal.
      void TestOrdinalConstructor();

      //! Tests the constant value representing the first Sequence.
      void TestFirstSequence();

      //! Tests the constant value representing the last Sequence.
      void TestLastSequence();

      //! Tests the less than operator.
      void TestLessThanOperator();

      //! Tests the less than or equal operator.
      void TestLessThanOrEqualOperator();

      //! Tests the equals operator.
      void TestEqualsOperator();

      //! Tests the not equals operator.
      void TestNotEqualsOperator();

      //! Tests the greater than or equal operator.
      void TestGreaterThanOrEqualOperator();

      //! Tests the greater than operator.
      void TestGreaterThanOperator();

      //! Tests incrementing a Sequence.
      void TestIncrement();

      //! Tests decrementing a Sequence.
      void TestDecrement();

      //! Tests encoding a timestamp into a Sequence.
      void TestEncodingTimestamp();

    private:
      CPPUNIT_TEST_SUITE(SequenceTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestOrdinalConstructor);
        CPPUNIT_TEST(TestFirstSequence);
        CPPUNIT_TEST(TestLastSequence);
        CPPUNIT_TEST(TestLessThanOperator);
        CPPUNIT_TEST(TestLessThanOrEqualOperator);
        CPPUNIT_TEST(TestEqualsOperator);
        CPPUNIT_TEST(TestNotEqualsOperator);
        CPPUNIT_TEST(TestGreaterThanOrEqualOperator);
        CPPUNIT_TEST(TestGreaterThanOperator);
        CPPUNIT_TEST(TestIncrement);
        CPPUNIT_TEST(TestDecrement);
        CPPUNIT_TEST(TestEncodingTimestamp);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
