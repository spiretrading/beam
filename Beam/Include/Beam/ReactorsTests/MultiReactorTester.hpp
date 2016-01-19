#ifndef BEAM_MULTIREACTORTESTER_HPP
#define BEAM_MULTIREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*! \class MultiReactorTester
      \brief Tests the MultiReactor class.
   */
  class MultiReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests no parameters.
      void TestNoParameters();

      //! Tests a single parameter.
      void TestSingleParameter();

      //! Tests two parameters.
      void TestTwoParameters();

    private:
      CPPUNIT_TEST_SUITE(MultiReactorTester);
        CPPUNIT_TEST(TestNoParameters);
        CPPUNIT_TEST(TestSingleParameter);
        CPPUNIT_TEST(TestTwoParameters);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
