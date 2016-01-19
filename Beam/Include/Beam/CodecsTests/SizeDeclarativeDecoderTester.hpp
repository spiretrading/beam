#ifndef BEAM_SIZEDECLARATIVEDECODERTESTER_HPP
#define BEAM_SIZEDECLARATIVEDECODERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class SizeDeclarativeDecoderTester
      \brief Tests the SizeDeclarativeDecoder class.
   */
  class SizeDeclarativeDecoderTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests decoding an empty message from a Buffer to a Buffer.
      void TestEmptyDecodeFromBufferToBuffer();

      //! Tests decoding a message from a Buffer to a Buffer.
      void TestDecodeFromBufferToBuffer();

      //! Tests decoding an empty message from a Buffer to a pointer.
      void TestEmptyDecodeFromBufferToPointer();

    private:
      CPPUNIT_TEST_SUITE(SizeDeclarativeDecoderTester);
        CPPUNIT_TEST(TestEmptyDecodeFromBufferToBuffer);
        CPPUNIT_TEST(TestDecodeFromBufferToBuffer);
        CPPUNIT_TEST(TestEmptyDecodeFromBufferToPointer);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
