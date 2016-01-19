#ifndef BEAM_TOKENPARSERTESTER_HPP
#define BEAM_TOKENPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ParsersTests/ParsersTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Parsers {
namespace Tests {

  /*! \class TokenParserTester
      \brief Tests the TokenParser class.
   */
  class TokenParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Test parsing a token with no leading spaces.
      void TestNoSpaceToken();

      //! Test parsing a single token.
      void TestOneToken();

      //! Test parsing two tokens.
      void TestTwoTokens();

      //! Test parsing a star of tokens.
      void TestStarTokens();

      //! Test chaining the token parser.
      void TestChainingTokens();

    private:
      CPPUNIT_TEST_SUITE(TokenParserTester);
        CPPUNIT_TEST(TestNoSpaceToken);
        CPPUNIT_TEST(TestOneToken);
        CPPUNIT_TEST(TestTwoTokens);
        CPPUNIT_TEST(TestStarTokens);
        CPPUNIT_TEST(TestChainingTokens);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
