#ifndef BEAM_CODEDWRITERTESTER_HPP
#define BEAM_CODEDWRITERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class CodedWriterTester
      \brief Tests the CodedWriter class.
   */
  class CodedWriterTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests writing a single byte.
      void TestSingleByte();

      //! Tests writing a message.
      void TestWrite();

    private:
      CPPUNIT_TEST_SUITE(CodedWriterTester);
        CPPUNIT_TEST(TestSingleByte);
        CPPUNIT_TEST(TestWrite);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
