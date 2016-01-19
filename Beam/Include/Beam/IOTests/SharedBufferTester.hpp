#ifndef BEAM_SHAREDBUFFERTESTER_HPP
#define BEAM_SHAREDBUFFERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IOTests/IOTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {
namespace Tests {

  /*! \class SharedBufferTester
      \brief Tests the SharedBuffer class.
   */
  class SharedBufferTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests creating an empty Buffer.
      void TestCreateEmpty();

      //! Tests creating a Buffer with an initial size.
      void TestCreateInitialSize();

      //! Tests growing an empty Buffer.
      void TestGrowingEmptyBuffer();

      //! Tests growing a initial sized Buffer.
      void TestGrowingInitialSizedBuffer();

      //! Tests copying a Buffer.
      void TestCopy();

      //! Tests appending to a Buffer.
      void TestAppend();

      //! Tests reseting a Buffer.
      void TestReset();

      //! Tests copy on write semantics by appending to the original Buffer.
      void TestCopyOnWriteWithAppendToOriginal();

      //! Tests copy on write semantics by appending to the copied Buffer.
      void TestCopyOnWriteWithAppendToCopy();

    private:
      CPPUNIT_TEST_SUITE(SharedBufferTester);
        CPPUNIT_TEST(TestCreateEmpty);
        CPPUNIT_TEST(TestCreateInitialSize);
        CPPUNIT_TEST(TestGrowingEmptyBuffer);
        CPPUNIT_TEST(TestGrowingInitialSizedBuffer);
        CPPUNIT_TEST(TestCopy);
        CPPUNIT_TEST(TestAppend);
        CPPUNIT_TEST(TestReset);
        CPPUNIT_TEST(TestCopyOnWriteWithAppendToOriginal);
        CPPUNIT_TEST(TestCopyOnWriteWithAppendToCopy);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
