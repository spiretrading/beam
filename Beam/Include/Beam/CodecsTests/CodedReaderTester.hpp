#ifndef BEAM_CODEDREADERTESTER_HPP
#define BEAM_CODEDREADERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class CodedReaderTester
      \brief Tests the CodedReader class.
   */
  class CodedReaderTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests reading from an empty source.
      void TestEmpty();

      //! Tests reading a single byte.
      void TestSingleByte();

      //! Tests reading a message.
      void TestRead();

      //! Tests reading a portion of a message.
      void TestReadSome();

    private:
      CPPUNIT_TEST_SUITE(CodedReaderTester);
        CPPUNIT_TEST(TestEmpty);
        CPPUNIT_TEST(TestSingleByte);
        CPPUNIT_TEST(TestRead);
        CPPUNIT_TEST(TestReadSome);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
