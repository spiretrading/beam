#ifndef BEAM_QUEUE_REACTOR_TESTER_HPP
#define BEAM_QUEUE_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class QueueReactorTester
      \brief Tests the QueueReactor class.
   */
  class QueueReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a Queue that immediately breaks without publishing any values.
      void TestEmptyQueue();

    private:
      CPPUNIT_TEST_SUITE(QueueReactorTester);
        CPPUNIT_TEST(TestEmptyQueue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
