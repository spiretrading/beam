#ifndef BEAM_NATIVEVALUETESTER_HPP
#define BEAM_NATIVEVALUETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class NativeValueTester
      \brief Tests the NativeValue class.
   */
  class NativeValueTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests making a NativeValue.
      void TestMakeNativeValue();

      //! Tests an int NativeValue.
      void TestInt();

      //! Tests a decimal NativeValue.
      void TestDecimal();

      //! Tests a string NativeValue.
      void TestString();

    private:
      CPPUNIT_TEST_SUITE(NativeValueTester);
        CPPUNIT_TEST(TestMakeNativeValue);
        CPPUNIT_TEST(TestInt);
        CPPUNIT_TEST(TestDecimal);
        CPPUNIT_TEST(TestString);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
