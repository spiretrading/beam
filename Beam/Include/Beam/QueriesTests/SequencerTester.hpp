#ifndef BEAM_SEQUENCERTESTER_HPP
#define BEAM_SEQUENCERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SequencerTester
      \brief Tests the Sequencer class.
   */
  class SequencerTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests sequencing the very first value in a time series.
      void TestFirstSequence();

    private:
      CPPUNIT_TEST_SUITE(SequencerTester);
        CPPUNIT_TEST(TestFirstSequence);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
