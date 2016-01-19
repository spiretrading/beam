#ifndef BEAM_PIPEDREADERWRITERTESTER_HPP
#define BEAM_PIPEDREADERWRITERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/IO.hpp"
#include "Beam/IOTests/IOTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {
namespace Tests {

  /*! \class PipedReaderWriterTester
      \brief Tests the PipedReader and PipedWriter classes.
   */
  class PipedReaderWriterTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a single write followed by a read.
      void TestWriteThenRead();

    private:
      CPPUNIT_TEST_SUITE(PipedReaderWriterTester);
        CPPUNIT_TEST(TestWriteThenRead);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
