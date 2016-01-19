#ifndef BEAM_SIZEDECLARATIVEREADERTESTER_HPP
#define BEAM_SIZEDECLARATIVEREADERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/IO.hpp"
#include "Beam/IOTests/IOTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {
namespace Tests {

  /*! \class SizeDeclarativeReaderTester
      \brief Tests the SizeDeclarativeReader class.
   */
  class SizeDeclarativeReaderTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Test reading from an empty data source.
      void TestEmptySource();

      //! Test reading a message in one single read.
      void TestSingleReadMessage();

      //! Test reading a message in two reads.
      void TestMultiReadMessage();

    private:
      CPPUNIT_TEST_SUITE(SizeDeclarativeReaderTester);
        CPPUNIT_TEST(TestEmptySource);
        CPPUNIT_TEST(TestSingleReadMessage);
        CPPUNIT_TEST(TestMultiReadMessage);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
