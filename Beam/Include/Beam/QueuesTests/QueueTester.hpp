#ifndef BEAM_QUEUETESTER_HPP
#define BEAM_QUEUETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueuesTests/QueuesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Tests {

  /*! \class QueueTester
      \brief Tests the Queue class.
   */
  class QueueTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests breaking the Queue.
      void TestBreak();

    private:
      CPPUNIT_TEST_SUITE(QueueTester);
        CPPUNIT_TEST(TestBreak);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
