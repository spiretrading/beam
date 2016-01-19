#ifndef BEAM_BUFFEREDDATASTORETESTER_HPP
#define BEAM_BUFFEREDDATASTORETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class BufferedDataStoreTester
      \brief Tests the BufferedDataStore class.
   */
  class BufferedDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests storing and loading a value.
      void TestStoreAndLoad();

      //! Tests loading a series of values that span from the destination to
      //! the buffer.
      void TestHeadSpanningLoad();

      //! Tests loading a series of values that span from the buffer to
      //! the destination.
      void TestTailSpanningLoad();

    private:
      CPPUNIT_TEST_SUITE(BufferedDataStoreTester);
        CPPUNIT_TEST(TestStoreAndLoad);
        CPPUNIT_TEST(TestHeadSpanningLoad);
        CPPUNIT_TEST(TestTailSpanningLoad);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
