#ifndef BEAM_INDEXEDVALUETESTER_HPP
#define BEAM_INDEXEDVALUETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class IndexedValueTester
      \brief Tests the IndexedValue class.
   */
  class IndexedValueTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests constructing with a value and index.
      void TestValueAndSequenceConstructor();

      //! Tests implicitly converting an IndexedValue to it's value.
      void TestDereference();

      //! Tests the equals operator.
      void TestEqualsOperator();

      //! Tests the not equals operator.
      void TestNotEqualsOperator();

      //! Tests the MakeIndexedValue builder.
      void TestMakeIndexedValue();

    private:
      CPPUNIT_TEST_SUITE(IndexedValueTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestValueAndSequenceConstructor);
        CPPUNIT_TEST(TestDereference);
        CPPUNIT_TEST(TestEqualsOperator);
        CPPUNIT_TEST(TestNotEqualsOperator);
        CPPUNIT_TEST(TestMakeIndexedValue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
