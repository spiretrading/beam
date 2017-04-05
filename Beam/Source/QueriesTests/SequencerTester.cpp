#include "Beam/QueriesTests/SequencerTester.hpp"
#include "Beam/Queries/Sequencer.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void SequencerTester::TestFirstSequence() {
  Sequencer sequencer{Sequence::First()};
  ptime timestampA{date{2016, 7, 31}, hours(17) + minutes(30)};
  auto sequenceA = sequencer.IncrementNextSequence(timestampA);
  CPPUNIT_ASSERT(sequenceA == EncodeTimestamp(timestampA, Sequence::First()));
  auto timestampB = timestampA += days(1);
  auto sequenceB = sequencer.IncrementNextSequence(timestampB);
  CPPUNIT_ASSERT(sequenceB == EncodeTimestamp(timestampB, Sequence::First()));
}
