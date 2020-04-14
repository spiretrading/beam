#include <doctest/doctest.h>
#include "Beam/Queries/Sequencer.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("Sequencer") {
  TEST_CASE("first_sequence") {
    auto sequencer = Sequencer(Beam::Queries::Sequence::First());
    auto timestampA = ptime(date(2016, 7, 31), hours(17) + minutes(30));
    auto sequenceA = sequencer.IncrementNextSequence(timestampA);
    REQUIRE(sequenceA == EncodeTimestamp(timestampA,
      Beam::Queries::Sequence::First()));
    auto timestampB = timestampA += days(1);
    auto sequenceB = sequencer.IncrementNextSequence(timestampB);
    REQUIRE(sequenceB == EncodeTimestamp(timestampB,
      Beam::Queries::Sequence::First()));
  }
}
