#ifndef BEAM_AGGREGATEREACTORTESTER_HPP
#define BEAM_AGGREGATEREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class AggregateReactorTester
      \brief Tests the AggregateReactor class.
   */
  class AggregateReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests using a producer that does not produce any Reactor.
      void TestNoneProducer();

      //! Tests two Reactors simultaneously updating.
      void TestSimultaneousUpdate();

    private:
      CPPUNIT_TEST_SUITE(AggregateReactorTester);
        CPPUNIT_TEST(TestNoneProducer);
        CPPUNIT_TEST(TestSimultaneousUpdate);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
