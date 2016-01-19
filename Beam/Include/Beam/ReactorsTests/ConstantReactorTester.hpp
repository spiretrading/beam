#ifndef BEAM_CONSTANTREACTORTESTER_HPP
#define BEAM_CONSTANTREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class ConstantReactorTester
      \brief Tests the ConstantReactor class.
   */
  class ConstantReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests an int ConstantReactor.
      void TestInt();

      //! Tests a decimal ConstantReactor.
      void TestDecimal();

      //! Tests a string ConstantReactor.
      void TestString();

    private:
      CPPUNIT_TEST_SUITE(ConstantReactorTester);
        CPPUNIT_TEST(TestInt);
        CPPUNIT_TEST(TestDecimal);
        CPPUNIT_TEST(TestString);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
