#ifndef BEAM_SEQUENCEDVALUETESTER_HPP
#define BEAM_SEQUENCEDVALUETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SequencedValueTester
      \brief Tests the SequencedValue class.
   */
  class SequencedValueTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests constructing with a value and Sequence.
      void TestValueAndSequenceConstructor();

      //! Tests implicitly converting a SequencedValue to it's value.
      void TestDereference();

      //! Tests the equals operator.
      void TestEqualsOperator();

      //! Tests the not equals operator.
      void TestNotEqualsOperator();

      //! Tests the MakeSequencedValue builder.
      void TestMakeSequencedValue();

    private:
      CPPUNIT_TEST_SUITE(SequencedValueTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestValueAndSequenceConstructor);
        CPPUNIT_TEST(TestDereference);
        CPPUNIT_TEST(TestEqualsOperator);
        CPPUNIT_TEST(TestNotEqualsOperator);
        CPPUNIT_TEST(TestMakeSequencedValue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
