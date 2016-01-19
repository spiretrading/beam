#ifndef BEAM_SWITCHREACTORTESTER_HPP
#define BEAM_SWITCHREACTORTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class SwitchReactorTester
      \brief Tests the SwitchReactor class.
   */
  class SwitchReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests using a producer that does not produce any Reactor.
      void TestNoneProducer();

      //! Tests a producer that results in an exception and then completes.
      void TestProducerExceptionCompletion();

    private:
      CPPUNIT_TEST_SUITE(SwitchReactorTester);
        CPPUNIT_TEST(TestNoneProducer);
        CPPUNIT_TEST(TestProducerExceptionCompletion);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
