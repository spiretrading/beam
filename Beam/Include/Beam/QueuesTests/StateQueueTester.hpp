#ifndef BEAM_STATEQUEUETESTER_HPP
#define BEAM_STATEQUEUETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueuesTests/QueuesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tests {

  /*! \class StateQueueTester
      \brief Tests the StateQueue class.
   */
  class StateQueueTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests breaking the Queue.
      void TestBreak();

    private:
      CPPUNIT_TEST_SUITE(StateQueueTester);
        CPPUNIT_TEST(TestBreak);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
