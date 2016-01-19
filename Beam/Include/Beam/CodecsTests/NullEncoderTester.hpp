#ifndef BEAM_NULLENCODERTESTER_HPP
#define BEAM_NULLENCODERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class NullEncoderTester
      \brief Tests the NullEncoder class.
   */
  class NullEncoderTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests encoding an empty message from a Buffer to a Buffer.
      void TestEmptyEncodeFromBufferToBuffer();

      //! Tests encoding an empty message from a Buffer to a Buffer in place.
      void TestEmptyEncodeFromBufferToBufferInPlace();

      //! Tests encoding a message from a Buffer to a Buffer.
      void TestEncodeFromBufferToBuffer();

      //! Tests encoding a message from a Buffer to a Buffer in place.
      void TestEncodeFromBufferToBufferInPlace();

      //! Tests encoding an empty message from a Buffer to a pointer.
      void TestEmptyEncodeFromBufferToPointer();

      //! Tests encoding an empty message from a Buffer to a pointer in place.
      void TestEmptyEncodeFromBufferToPointerInPlace();

      //! Tests encoding a message from a Buffer to a pointer.
      void TestEncodeFromBufferToPointer();

      //! Tests encoding a message from a Buffer to a pointer in place.
      void TestEncodeFromBufferToPointerInPlace();

      //! Tests encoding a message from a Buffer to a pointer where the
      //! destination is unable to contain the message.
      void TestEncodeFromBufferToSmallerPointer();

      //! Tests encoding an empty message from a pointer to a Buffer.
      void TestEmptyEncodeFromPointerToBuffer();

      //! Tests encoding an empty message from a pointer to a Buffer in place.
      void TestEmptyEncodeFromPointerToBufferInPlace();

      //! Tests encoding a message from a pointer to a Buffer.
      void TestEncodeFromPointerToBuffer();

      //! Tests encoding a message from a pointer to a Buffer in place.
      void TestEncodeFromPointerToBufferInPlace();

      //! Tests encoding an empty message from a pointer to a pointer.
      void TestEmptyEncodeFromPointerToPointer();

      //! Tests encoding an empty message from a pointer to a pointer in place.
      void TestEmptyEncodeFromPointerToPointerInPlace();

      //! Tests encoding a message from a pointer to a pointer.
      void TestEncodeFromPointerToPointer();

      //! Tests encoding a message from a pointer to a pointer in place.
      void TestEncodeFromPointerToPointerInPlace();

      //! Tests encoding a message from a pointer to a pointer where the
      //! destination is unable to contain the message.
      void TestEncodeFromPointerToSmallerPointer();

    private:
      CPPUNIT_TEST_SUITE(NullEncoderTester);
        CPPUNIT_TEST(TestEmptyEncodeFromBufferToBuffer);
        CPPUNIT_TEST(TestEmptyEncodeFromBufferToBufferInPlace);
        CPPUNIT_TEST(TestEncodeFromBufferToBuffer);
        CPPUNIT_TEST(TestEncodeFromBufferToBufferInPlace);
        CPPUNIT_TEST(TestEmptyEncodeFromBufferToPointer);
        CPPUNIT_TEST(TestEmptyEncodeFromBufferToPointerInPlace);
        CPPUNIT_TEST(TestEncodeFromBufferToPointer);
        CPPUNIT_TEST(TestEncodeFromBufferToPointerInPlace);
        CPPUNIT_TEST(TestEncodeFromBufferToSmallerPointer);
        CPPUNIT_TEST(TestEmptyEncodeFromPointerToBuffer);
        CPPUNIT_TEST(TestEmptyEncodeFromPointerToBufferInPlace);
        CPPUNIT_TEST(TestEncodeFromPointerToBuffer);
        CPPUNIT_TEST(TestEncodeFromPointerToBufferInPlace);
        CPPUNIT_TEST(TestEmptyEncodeFromPointerToPointer);
        CPPUNIT_TEST(TestEmptyEncodeFromPointerToPointerInPlace);
        CPPUNIT_TEST(TestEncodeFromPointerToPointer);
        CPPUNIT_TEST(TestEncodeFromPointerToPointerInPlace);
        CPPUNIT_TEST(TestEncodeFromPointerToSmallerPointer);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
