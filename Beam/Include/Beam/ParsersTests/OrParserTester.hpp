#ifndef BEAM_ORPARSERTESTER_HPP
#define BEAM_ORPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class OrParserTester
      \brief Tests the OrParser class.
   */
  class OrParserTester : public CPPUNIT_NS::TestFixture {
    public:

      void TestChainingNoNullParsersWithNoDuplicateTypes();

      void TestChainingNoNullParsersWithDuplicateTypes();

    private:
      CPPUNIT_TEST_SUITE(OrParserTester);
        CPPUNIT_TEST(TestChainingNoNullParsersWithNoDuplicateTypes);
        CPPUNIT_TEST(TestChainingNoNullParsersWithDuplicateTypes);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
