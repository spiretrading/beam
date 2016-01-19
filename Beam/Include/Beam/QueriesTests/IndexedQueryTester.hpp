#ifndef BEAM_INDEXEDQUERYTESTER_HPP
#define BEAM_INDEXEDQUERYTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class IndexedQueryTester
      \brief Tests the IndexedQuery class.
   */
  class IndexedQueryTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests the default constructor.
      void TestDefaultConstructor();

      //! Tests the constructor specifying an index.
      void TestIndexConstructor();

      //! Tests setting an index.
      void TestSetIndex();

    private:
      CPPUNIT_TEST_SUITE(IndexedQueryTester);
        CPPUNIT_TEST(TestDefaultConstructor);
        CPPUNIT_TEST(TestIndexConstructor);
        CPPUNIT_TEST(TestSetIndex);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
