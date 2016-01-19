#ifndef BEAM_NATIVEDATATYPETESTER_HPP
#define BEAM_NATIVEDATATYPETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class NativeDataTypeTester
      \brief Tests the NativeDataType class.
   */
  class NativeDataTypeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests an int NativeDataType.
      void TestInt();

      //! Tests a string NativeDataType.
      void TestString();

    private:
      CPPUNIT_TEST_SUITE(NativeDataTypeTester);
        CPPUNIT_TEST(TestInt);
        CPPUNIT_TEST(TestString);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
