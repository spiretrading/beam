#ifndef BEAM_NULLDECODERTESTER_HPP
#define BEAM_NULLDECODERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class NullDecoderTester
      \brief Tests the NullDecoder class.
   */
  class NullDecoderTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests decoding an empty message from a Buffer to a Buffer.
      void TestEmptyDecodeFromBufferToBuffer();

      //! Tests decoding an empty message from a Buffer to a Buffer in place.
      void TestEmptyDecodeFromBufferToBufferInPlace();

      //! Tests decoding a message from a Buffer to a Buffer.
      void TestDecodeFromBufferToBuffer();

      //! Tests decoding a message from a Buffer to a Buffer in place.
      void TestDecodeFromBufferToBufferInPlace();

      //! Tests decoding an empty message from a Buffer to a pointer.
      void TestEmptyDecodeFromBufferToPointer();

      //! Tests decoding an empty message from a Buffer to a pointer in place.
      void TestEmptyDecodeFromBufferToPointerInPlace();

      //! Tests decoding a message from a Buffer to a pointer.
      void TestDecodeFromBufferToPointer();

      //! Tests decoding a message from a Buffer to a pointer in place.
      void TestDecodeFromBufferToPointerInPlace();

      //! Tests decoding a message from a Buffer to a pointer where the
      //! destination is unable to contain the message.
      void TestDecodeFromBufferToSmallerPointer();

      //! Tests decoding an empty message from a pointer to a Buffer.
      void TestEmptyDecodeFromPointerToBuffer();

      //! Tests decoding an empty message from a pointer to a Buffer in place.
      void TestEmptyDecodeFromPointerToBufferInPlace();

      //! Tests decoding a message from a pointer to a Buffer.
      void TestDecodeFromPointerToBuffer();

      //! Tests decoding a message from a pointer to a Buffer in place.
      void TestDecodeFromPointerToBufferInPlace();

      //! Tests decoding an empty message from a pointer to a pointer.
      void TestEmptyDecodeFromPointerToPointer();

      //! Tests decoding an empty message from a pointer to a pointer in place.
      void TestEmptyDecodeFromPointerToPointerInPlace();

      //! Tests decoding a message from a pointer to a pointer.
      void TestDecodeFromPointerToPointer();

      //! Tests decoding a message from a pointer to a pointer in place.
      void TestDecodeFromPointerToPointerInPlace();

      //! Tests decoding a message from a pointer to a pointer where the
      //! destination is unable to contain the message.
      void TestDecodeFromPointerToSmallerPointer();

    private:
      CPPUNIT_TEST_SUITE(NullDecoderTester);
        CPPUNIT_TEST(TestEmptyDecodeFromBufferToBuffer);
        CPPUNIT_TEST(TestEmptyDecodeFromBufferToBufferInPlace);
        CPPUNIT_TEST(TestDecodeFromBufferToBuffer);
        CPPUNIT_TEST(TestDecodeFromBufferToBufferInPlace);
        CPPUNIT_TEST(TestEmptyDecodeFromBufferToPointer);
        CPPUNIT_TEST(TestEmptyDecodeFromBufferToPointerInPlace);
        CPPUNIT_TEST(TestDecodeFromBufferToPointer);
        CPPUNIT_TEST(TestDecodeFromBufferToPointerInPlace);
        CPPUNIT_TEST(TestDecodeFromBufferToSmallerPointer);
        CPPUNIT_TEST(TestEmptyDecodeFromPointerToBuffer);
        CPPUNIT_TEST(TestEmptyDecodeFromPointerToBufferInPlace);
        CPPUNIT_TEST(TestDecodeFromPointerToBuffer);
        CPPUNIT_TEST(TestDecodeFromPointerToBufferInPlace);
        CPPUNIT_TEST(TestEmptyDecodeFromPointerToPointer);
        CPPUNIT_TEST(TestEmptyDecodeFromPointerToPointerInPlace);
        CPPUNIT_TEST(TestDecodeFromPointerToPointer);
        CPPUNIT_TEST(TestDecodeFromPointerToPointerInPlace);
        CPPUNIT_TEST(TestDecodeFromPointerToSmallerPointer);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
