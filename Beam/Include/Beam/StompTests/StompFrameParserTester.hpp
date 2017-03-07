#ifndef BEAM_STOMPFRAMEPARSERTESTER_HPP
#define BEAM_STOMPFRAMEPARSERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/StompTests/StompTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Stomp {
namespace Tests {

  /*! \class StompFrameParserTester
      \brief Tests the StompFrameParser class.
   */
  class StompFrameParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing a frame with just a new line character.
      void TestNewLineCharacter();

      //! Tests parsing a frame with new line and carriage return characters.
      void TestCarriageAndNewLineCharacter();

      //! Tests parsing EOL frames.
      void TestEolFrame();

      //! Test escaping characters in headers.
      void TestEscapeCharacters();

    private:
      CPPUNIT_TEST_SUITE(StompFrameParserTester);
        CPPUNIT_TEST(TestNewLineCharacter);
        CPPUNIT_TEST(TestCarriageAndNewLineCharacter);
        CPPUNIT_TEST(TestEolFrame);
        CPPUNIT_TEST(TestEscapeCharacters);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
