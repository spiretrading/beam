#ifndef BEAM_ENUMERATORPARSERTESTER_HPP
#define BEAM_ENUMERATORPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class EnumeratorParserTester
      \brief Tests the EnumeratorParser class.
   */
  class EnumeratorParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests reading an Enumerator.
      void TestReadEnumerator();

    private:
      CPPUNIT_TEST_SUITE(EnumeratorParserTester);
        CPPUNIT_TEST(TestReadEnumerator);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
