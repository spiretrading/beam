#ifndef BEAM_LOCALDATASTORETESTER_HPP
#define BEAM_LOCALDATASTORETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class LocalDataStoreTester
      \brief Tests the LocalDataStore class.
   */
  class LocalDataStoreTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests storing and loading a value.
      void TestStoreAndLoad();

      //! Tests loading all values.
      void TestLoadAll();

    private:
      CPPUNIT_TEST_SUITE(LocalDataStoreTester);
        CPPUNIT_TEST(TestStoreAndLoad);
        CPPUNIT_TEST(TestLoadAll);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
