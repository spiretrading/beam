#include "Beam/QueriesTests/RangeTester.hpp"
#include <boost/type_traits.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "Beam/Queries/Range.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void RangeTester::TestDefaultConstructor() {
  Range range;
  CPPUNIT_ASSERT(range.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(range.GetEnd() == Sequence::First());
}

void RangeTester::TestEmptyRangeConstructors() {
  Range negInfinRange(neg_infin, neg_infin);
  CPPUNIT_ASSERT(negInfinRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(negInfinRange.GetEnd() == Sequence::First());
  Range invalidStartDateRange(not_a_date_time, Sequence(5));
  CPPUNIT_ASSERT(invalidStartDateRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(invalidStartDateRange.GetEnd() == Sequence::First());
  Range invalidEndDateRange(Sequence(5), not_a_date_time);
  CPPUNIT_ASSERT(invalidEndDateRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(invalidEndDateRange.GetEnd() == Sequence::First());
  Range invalidDateRange(not_a_date_time, not_a_date_time);
  CPPUNIT_ASSERT(invalidDateRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(invalidDateRange.GetEnd() == Sequence::First());
}

void RangeTester::TestTotalRangeConstructors() {
  Range totalDateRange(neg_infin, pos_infin);
  CPPUNIT_ASSERT(totalDateRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(totalDateRange.GetEnd() == Sequence::Last());
  Range totalSequenceRange(Sequence::First(), Sequence::Last());
  CPPUNIT_ASSERT(totalSequenceRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(totalSequenceRange.GetEnd() == Sequence::Last());
  Range totalDateSequenceRange(neg_infin, Sequence::Last());
  CPPUNIT_ASSERT(totalDateSequenceRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(totalDateSequenceRange.GetEnd() == Sequence::Last());
  Range totalSequenceDateRange(Sequence::First(), pos_infin);
  CPPUNIT_ASSERT(totalSequenceDateRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(totalSequenceDateRange.GetEnd() == Sequence::Last());
}

void RangeTester::TestOutOfOrderConstructors() {
  Range posToNegRange(pos_infin, neg_infin);
  CPPUNIT_ASSERT(posToNegRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(posToNegRange.GetEnd() == Sequence::First());
  Range sequenceRange(Sequence(5), Sequence(4));
  CPPUNIT_ASSERT(sequenceRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(sequenceRange.GetEnd() == Sequence::First());
  Range outOfOrderDateRange(ptime(date(2000, Jan, 13)),
    ptime(date(1999, Jan, 13)));
  CPPUNIT_ASSERT(outOfOrderDateRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(outOfOrderDateRange.GetEnd() == Sequence::First());
  Range outOfOrderPosInfinRange(pos_infin, ptime(date(1999, Jan, 13)));
  CPPUNIT_ASSERT(outOfOrderPosInfinRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(outOfOrderPosInfinRange.GetEnd() == Sequence::First());
  Range outOfOrderNegInfinRange(ptime(date(1999, Jan, 13)), neg_infin);
  CPPUNIT_ASSERT(outOfOrderNegInfinRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(outOfOrderNegInfinRange.GetEnd() == Sequence::First());
}

void RangeTester::TestSequenceConstructors() {
  Range singleRange(Sequence(1), Sequence(1));
  CPPUNIT_ASSERT(singleRange.GetStart() == Sequence(1));
  CPPUNIT_ASSERT(singleRange.GetEnd() == Sequence(1));
  Range openRange(Sequence(1), Sequence(2));
  CPPUNIT_ASSERT(openRange.GetStart() == Sequence(1));
  CPPUNIT_ASSERT(openRange.GetEnd() == Sequence(2));
  Range snapshotRealTimeRange(Sequence(1), Sequence::Last());
  CPPUNIT_ASSERT(snapshotRealTimeRange.GetStart() == Sequence(1));
  CPPUNIT_ASSERT(snapshotRealTimeRange.GetEnd() == Sequence::Last());
  Range snapshotRange(Sequence::First(), Sequence(1));
  CPPUNIT_ASSERT(snapshotRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(snapshotRange.GetEnd() == Sequence(1));
}

void RangeTester::TestDateConstructors() {
  Range singleRange(ptime(date(1984, May, 6)), ptime(date(1984, May, 6)));
  CPPUNIT_ASSERT(singleRange.GetStart() == ptime(date(1984, May, 6)));
  CPPUNIT_ASSERT(singleRange.GetEnd() == ptime(date(1984, May, 6)));
  Range openRange(ptime(date(1984, May, 6)), ptime(date(2014, May, 6)));
  CPPUNIT_ASSERT(openRange.GetStart() == ptime(date(1984, May, 6)));
  CPPUNIT_ASSERT(openRange.GetEnd() == ptime(date(2014, May, 6)));
  Range snapshotRealTimeRange(ptime(date(2014, May, 6)), pos_infin);
  CPPUNIT_ASSERT(snapshotRealTimeRange.GetStart() == ptime(date(2014, May, 6)));
  CPPUNIT_ASSERT(snapshotRealTimeRange.GetEnd() == Sequence::Last());
  Range snapshotRange(neg_infin, ptime(date(2014, May, 6)));
  CPPUNIT_ASSERT(snapshotRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(snapshotRange.GetEnd() == ptime(date(2014, May, 6)));
}

void RangeTester::TestMixedConstructors() {
  Range dateSequenceRange(ptime(date(1984, May, 6)), Sequence(5));
  CPPUNIT_ASSERT(dateSequenceRange.GetStart() == ptime(date(1984, May, 6)));
  CPPUNIT_ASSERT(dateSequenceRange.GetEnd() == Sequence(5));
  Range sequenceDateRange(Sequence(5), ptime(date(1984, May, 6)));
  CPPUNIT_ASSERT(sequenceDateRange.GetStart() == Sequence(5));
  CPPUNIT_ASSERT(sequenceDateRange.GetEnd() == ptime(date(1984, May, 6)));
  Range sequenceDateSnapshotRealTimeRange(Sequence(5), pos_infin);
  CPPUNIT_ASSERT(sequenceDateSnapshotRealTimeRange.GetStart() == Sequence(5));
  CPPUNIT_ASSERT(sequenceDateSnapshotRealTimeRange.GetEnd() ==
    Sequence::Last());
  Range dateSequenceSnapshotRealTimeRange(ptime(date(1984, May, 6)),
    Sequence::Last());
  CPPUNIT_ASSERT(dateSequenceSnapshotRealTimeRange.GetStart() ==
    ptime(date(1984, May, 6)));
  CPPUNIT_ASSERT(dateSequenceSnapshotRealTimeRange.GetEnd() ==
    Sequence::Last());
  Range sequenceDateSnapshotRange(Sequence::First(), ptime(date(1984, May, 6)));
  CPPUNIT_ASSERT(sequenceDateSnapshotRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(sequenceDateSnapshotRange.GetEnd() ==
    ptime(date(1984, May, 6)));
  Range dateSequenceSnapshotRange(neg_infin, Sequence(5));
  CPPUNIT_ASSERT(dateSequenceSnapshotRange.GetStart() == Sequence::First());
  CPPUNIT_ASSERT(dateSequenceSnapshotRange.GetEnd() == Sequence(5));
}

void RangeTester::TestEmptyRangeValue() {
  CPPUNIT_ASSERT(Range::Empty().GetStart() == Sequence::First());
  CPPUNIT_ASSERT(Range::Empty().GetEnd() == Sequence::First());
}

void RangeTester::TestTotalRangeValue() {
  CPPUNIT_ASSERT(Range::Total().GetStart() == Sequence::First());
  CPPUNIT_ASSERT(Range::Total().GetEnd() == Sequence::Last());
}

void RangeTester::TestRealTimeRangeValue() {
  CPPUNIT_ASSERT(Range::RealTime().GetStart() == Sequence::Present());
  CPPUNIT_ASSERT(Range::RealTime().GetEnd() == Sequence::Last());
}

void RangeTester::TestEqualsOperator() {
  CPPUNIT_ASSERT(Range(Sequence(1), Sequence(2)) ==
    Range(Sequence(1), Sequence(2)));
  CPPUNIT_ASSERT(!(Range(Sequence(1), Sequence(2)) ==
    Range(Sequence(2), Sequence(1))));
  CPPUNIT_ASSERT(Range(Sequence::First(), Sequence(1)) ==
    Range(neg_infin, Sequence(1)));
  CPPUNIT_ASSERT(Range(ptime(date(1984, May, 6)), Sequence(2)) ==
    Range(ptime(date(1984, May, 6)), Sequence(2)));
  CPPUNIT_ASSERT(!(Range(ptime(date(1984, May, 6)), Sequence(2)) ==
    Range(Sequence(2), ptime(date(1984, May, 6)))));
  CPPUNIT_ASSERT(Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))) ==
    Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))));
  CPPUNIT_ASSERT(!(Range(ptime(date(1984, May, 6)),
    ptime(date(1985, May, 6))) == Range(ptime(date(1985, May, 6)),
    ptime(date(1985, May, 6)))));
}

void RangeTester::TestNotEqualsOperator() {
  CPPUNIT_ASSERT(!(Range(Sequence(1), Sequence(2)) !=
    Range(Sequence(1), Sequence(2))));
  CPPUNIT_ASSERT(!(Range(Sequence::First(), Sequence(1)) !=
    Range(neg_infin, Sequence(1))));
  CPPUNIT_ASSERT(!(Range(ptime(date(1984, May, 6)), Sequence(2)) !=
    Range(ptime(date(1984, May, 6)), Sequence(2))));
  CPPUNIT_ASSERT(Range(ptime(date(1984, May, 6)), Sequence(2)) !=
    Range(Sequence(2), ptime(date(1984, May, 6))));
  CPPUNIT_ASSERT(!(Range(ptime(date(1984, May, 6)),
    ptime(date(1985, May, 6))) != Range(ptime(date(1984, May, 6)),
    ptime(date(1985, May, 6)))));
  CPPUNIT_ASSERT(Range(ptime(date(1984, May, 6)),
    ptime(date(1985, May, 6))) != Range(ptime(date(1985, May, 6)),
    ptime(date(1985, May, 6))));
}
