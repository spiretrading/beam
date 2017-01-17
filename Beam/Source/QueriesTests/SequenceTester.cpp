#include "Beam/QueriesTests/SequenceTester.hpp"
#include "Beam/Queries/Sequence.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void SequenceTester::TestDefaultConstructor() {
  Sequence sequence;
  CPPUNIT_ASSERT(sequence.GetOrdinal() == 0);
}

void SequenceTester::TestOrdinalConstructor() {
  Sequence sequenceA(1);
  CPPUNIT_ASSERT(sequenceA.GetOrdinal() == 1);
  Sequence sequenceB(5);
  CPPUNIT_ASSERT(sequenceB.GetOrdinal() == 5);
  Sequence sequenceC(std::numeric_limits<std::uint64_t>::max());
  CPPUNIT_ASSERT(sequenceC.GetOrdinal() ==
    std::numeric_limits<std::uint64_t>::max());
}

void SequenceTester::TestFirstSequence() {
  Sequence first = Sequence::First();
  CPPUNIT_ASSERT(first.GetOrdinal() == 0);
}

void SequenceTester::TestLastSequence() {
  Sequence last = Sequence::Last();
  CPPUNIT_ASSERT(last.GetOrdinal() ==
    std::numeric_limits<std::uint64_t>::max());
}

void SequenceTester::TestLessThanOperator() {
  CPPUNIT_ASSERT(!(Sequence(0) < Sequence(0)));
  CPPUNIT_ASSERT(Sequence(0) < Sequence(1));
  CPPUNIT_ASSERT(!(Sequence(1) < Sequence(0)));
  CPPUNIT_ASSERT(!(Sequence(1) < Sequence(1)));
}

void SequenceTester::TestLessThanOrEqualOperator() {
  CPPUNIT_ASSERT(Sequence(0) <= Sequence(0));
  CPPUNIT_ASSERT(Sequence(0) <= Sequence(1));
  CPPUNIT_ASSERT(!(Sequence(1) <= Sequence(0)));
  CPPUNIT_ASSERT(Sequence(1) <= Sequence(1));
}

void SequenceTester::TestEqualsOperator() {
  CPPUNIT_ASSERT(Sequence(0) == Sequence(0));
  CPPUNIT_ASSERT(!(Sequence(0) == Sequence(1)));
  CPPUNIT_ASSERT(!(Sequence(1) == Sequence(0)));
  CPPUNIT_ASSERT(Sequence(1) == Sequence(1));
}

void SequenceTester::TestNotEqualsOperator() {
  CPPUNIT_ASSERT(!(Sequence(0) != Sequence(0)));
  CPPUNIT_ASSERT(Sequence(0) != Sequence(1));
  CPPUNIT_ASSERT(Sequence(1) != Sequence(0));
  CPPUNIT_ASSERT(!(Sequence(1) != Sequence(1)));
}

void SequenceTester::TestGreaterThanOrEqualOperator() {
  CPPUNIT_ASSERT(Sequence(0) >= Sequence(0));
  CPPUNIT_ASSERT(!(Sequence(0) >= Sequence(1)));
  CPPUNIT_ASSERT(Sequence(1) >= Sequence(0));
  CPPUNIT_ASSERT(Sequence(1) >= Sequence(1));
}

void SequenceTester::TestGreaterThanOperator() {
  CPPUNIT_ASSERT(!(Sequence(0) > Sequence(0)));
  CPPUNIT_ASSERT(!(Sequence(0) > Sequence(1)));
  CPPUNIT_ASSERT(Sequence(1) > Sequence(0));
  CPPUNIT_ASSERT(!(Sequence(1) > Sequence(1)));
}

void SequenceTester::TestIncrement() {
  CPPUNIT_ASSERT(Increment(Sequence(0)) == Sequence(1));
  CPPUNIT_ASSERT(Increment(Sequence(1)) == Sequence(2));
  CPPUNIT_ASSERT(Increment(Sequence(2)) == Sequence(3));
  CPPUNIT_ASSERT(Increment(Sequence::Last()) == Sequence::Last());
}

void SequenceTester::TestDecrement() {
  CPPUNIT_ASSERT(Decrement(Sequence(1)) == Sequence(0));
  CPPUNIT_ASSERT(Decrement(Sequence(2)) == Sequence(1));
  CPPUNIT_ASSERT(Decrement(Sequence(3)) == Sequence(2));
  CPPUNIT_ASSERT(Decrement(Sequence::First()) == Sequence::First());
}

void SequenceTester::TestEncodingTimestamp() {
  ptime timestamp{date{1984, May, 6}, hours(12) + minutes(44) + seconds(53)};
  auto sequence = EncodeTimestamp(timestamp);
  Sequence::Ordinal encoding =
    static_cast<Sequence::Ordinal>(0b00111110000000101000110) <<
    (CHAR_BIT * sizeof(Sequence::Ordinal) - 23);
  CPPUNIT_ASSERT(encoding == sequence.GetOrdinal());
  CPPUNIT_ASSERT(DecodeTimestamp(Sequence{encoding}).date() ==
    timestamp.date());
}
