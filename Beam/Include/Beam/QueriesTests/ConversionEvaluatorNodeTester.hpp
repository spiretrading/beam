#ifndef BEAM_CONVERSIONEVALUATORNODETESTER_HPP
#define BEAM_CONVERSIONEVALUATORNODETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ConversionEvaluatorNodeTester
      \brief Tests the various evaluators used to convert values.
   */
  class ConversionEvaluatorNodeTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests casting an int to a double.
      void TestCastIntToDouble();

      //! Tests converting a char array to a string.
      void TestConvertCharArrayToString();

    private:
      CPPUNIT_TEST_SUITE(ConversionEvaluatorNodeTester);
        CPPUNIT_TEST(TestCastIntToDouble);
        CPPUNIT_TEST(TestConvertCharArrayToString);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
