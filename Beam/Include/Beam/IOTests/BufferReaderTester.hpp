#ifndef BEAM_BUFFERREADERTESTER_HPP
#define BEAM_BUFFERREADERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/IO.hpp"
#include "Beam/IOTests/IOTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {
namespace Tests {

  /*! \class BufferReaderTester
      \brief Tests the BufferReader class.
   */
  class BufferReaderTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests creating with empty data.
      void TestCreateEmpty();

      //! Tests reading all contents.
      void TestRead();

      //! Tests reading some contents to a Buffer.
      void TestReadSomeToBuffer();

      //! Tests reading some contents to a pointer.
      void TestReadSomeToPointer();

    private:
      CPPUNIT_TEST_SUITE(BufferReaderTester);
        CPPUNIT_TEST(TestCreateEmpty);
        CPPUNIT_TEST(TestRead);
        CPPUNIT_TEST(TestReadSomeToBuffer);
        CPPUNIT_TEST(TestReadSomeToPointer);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
