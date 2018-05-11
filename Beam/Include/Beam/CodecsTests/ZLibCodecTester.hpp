#ifndef BEAM_ZLIB_CODEC_TESTER_HPP
#define BEAM_ZLIB_CODEC_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  //! Tests the zlib Encoder and Decoder classes.
  class ZLibCodecTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the codec on an empty message.
      void TestEmptyMessage();

      //! Tests the codec on a simple message.
      void TestSimpleMessage();

    private:
      CPPUNIT_TEST_SUITE(ZLibCodecTester);
        CPPUNIT_TEST(TestEmptyMessage);
        CPPUNIT_TEST(TestSimpleMessage);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
