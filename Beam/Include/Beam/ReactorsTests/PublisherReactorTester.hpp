#ifndef BEAM_PUBLISHERREACTORTESTER_HPP
#define BEAM_PUBLISHERREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class PublisherReactorTester
      \brief Tests the PublisherReactor class.
   */
  class PublisherReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests updating a PublisherReactor where only a single value is
      //! published before being committed.
      void TestOneByOneUpdates();

      //! Tests updating a PublisherReactor where multiple values are
      //! published before being committed.
      void TestMultipleUpdates();

    private:
      CPPUNIT_TEST_SUITE(PublisherReactorTester);
        CPPUNIT_TEST(TestOneByOneUpdates);
        CPPUNIT_TEST(TestMultipleUpdates);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
