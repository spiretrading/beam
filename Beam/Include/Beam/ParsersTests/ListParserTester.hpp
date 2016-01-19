#ifndef BEAM_LISTPARSERTESTER_HPP
#define BEAM_LISTPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class ListParserTester
      \brief Tests the ListParser class.
   */
  class ListParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a list consisting of a single int.
      void TestSingleIntList();

      //! Tests a list consisting of two ints.
      void TestTwoIntList();

      //! Tests a list consisting of three ints.
      void TestThreeIntList();

    private:
      CPPUNIT_TEST_SUITE(ListParserTester);
        CPPUNIT_TEST(TestSingleIntList);
        CPPUNIT_TEST(TestTwoIntList);
        CPPUNIT_TEST(TestThreeIntList);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
