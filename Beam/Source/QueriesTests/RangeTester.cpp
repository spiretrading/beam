#include <boost/date_time/gregorian/gregorian.hpp>
#include <doctest/doctest.h>
#include "Beam/Queries/Range.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("Range") {
  TEST_CASE("default_constructor") {
    auto range = Range();
    REQUIRE(range.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(range.GetEnd() == Beam::Queries::Sequence::First());
  }

  TEST_CASE("empty_range_constructors") {
    auto negInfinRange = Range(neg_infin, neg_infin);
    REQUIRE(negInfinRange.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(negInfinRange.GetEnd() == Beam::Queries::Sequence::First());
    auto invalidStartDateRange = Range(not_a_date_time,
      Beam::Queries::Sequence(5));
    REQUIRE(invalidStartDateRange.GetStart() ==
      Beam::Queries::Sequence::First());
    REQUIRE(invalidStartDateRange.GetEnd() ==
      Beam::Queries::Sequence::First());
    auto invalidEndDateRange = Range(Beam::Queries::Sequence(5),
      not_a_date_time);
    REQUIRE(invalidEndDateRange.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(invalidEndDateRange.GetEnd() == Beam::Queries::Sequence::First());
    auto invalidDateRange = Range(not_a_date_time, not_a_date_time);
    REQUIRE(invalidDateRange.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(invalidDateRange.GetEnd() == Beam::Queries::Sequence::First());
  }

  TEST_CASE("total_range_constructors") {
    auto totalDateRange = Range(neg_infin, pos_infin);
    REQUIRE(totalDateRange.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(totalDateRange.GetEnd() == Beam::Queries::Sequence::Last());
    auto totalSequenceRange = Range(Beam::Queries::Sequence::First(),
      Beam::Queries::Sequence::Last());
    REQUIRE(totalSequenceRange.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(totalSequenceRange.GetEnd() == Beam::Queries::Sequence::Last());
    auto totalDateSequenceRange = Range(neg_infin,
      Beam::Queries::Sequence::Last());
    REQUIRE(totalDateSequenceRange.GetStart() ==
      Beam::Queries::Sequence::First());
    REQUIRE(totalDateSequenceRange.GetEnd() == Beam::Queries::Sequence::Last());
    auto totalSequenceDateRange = Range(Beam::Queries::Sequence::First(),
      pos_infin);
    REQUIRE(totalSequenceDateRange.GetStart() ==
      Beam::Queries::Sequence::First());
    REQUIRE(totalSequenceDateRange.GetEnd() == Beam::Queries::Sequence::Last());
  }

  TEST_CASE("out_of_order_constructors") {
    auto posToNegRange = Range(pos_infin, neg_infin);
    REQUIRE(posToNegRange.GetStart() == Beam::Queries::Sequence::Last());
    REQUIRE(posToNegRange.GetEnd() == Beam::Queries::Sequence::First());
    auto sequenceRange = Range(Beam::Queries::Sequence(5),
      Beam::Queries::Sequence(4));
    REQUIRE(sequenceRange.GetStart() == Beam::Queries::Sequence(5));
    REQUIRE(sequenceRange.GetEnd() == Beam::Queries::Sequence(4));
    auto outOfOrderDateRange = Range(ptime(date(2000, Jan, 13)),
      ptime(date(1999, Jan, 13)));
    REQUIRE(outOfOrderDateRange.GetStart() == ptime(date(2000, Jan, 13)));
    REQUIRE(outOfOrderDateRange.GetEnd() == ptime(date(1999, Jan, 13)));
    auto outOfOrderPosInfinRange = Range(pos_infin, ptime(date(1999, Jan, 13)));
    REQUIRE(outOfOrderPosInfinRange.GetStart() ==
      Beam::Queries::Sequence::Last());
    REQUIRE(outOfOrderPosInfinRange.GetEnd() == ptime(date(1999, Jan, 13)));
    auto outOfOrderNegInfinRange = Range(ptime(date(1999, Jan, 13)), neg_infin);
    REQUIRE(outOfOrderNegInfinRange.GetStart() == ptime(date(1999, Jan, 13)));
    REQUIRE(outOfOrderNegInfinRange.GetEnd() ==
      Beam::Queries::Sequence::First());
  }

  TEST_CASE("sequence_constructors") {
    auto singleRange = Range(Beam::Queries::Sequence(1),
      Beam::Queries::Sequence(1));
    REQUIRE(singleRange.GetStart() == Beam::Queries::Sequence(1));
    REQUIRE(singleRange.GetEnd() == Beam::Queries::Sequence(1));
    auto openRange = Range(Beam::Queries::Sequence(1),
      Beam::Queries::Sequence(2));
    REQUIRE(openRange.GetStart() == Beam::Queries::Sequence(1));
    REQUIRE(openRange.GetEnd() == Beam::Queries::Sequence(2));
    auto snapshotRealTimeRange = Range(Beam::Queries::Sequence(1),
      Beam::Queries::Sequence::Last());
    REQUIRE(snapshotRealTimeRange.GetStart() == Beam::Queries::Sequence(1));
    REQUIRE(snapshotRealTimeRange.GetEnd() == Beam::Queries::Sequence::Last());
    auto snapshotRange = Range(Beam::Queries::Sequence::First(),
      Beam::Queries::Sequence(1));
    REQUIRE(snapshotRange.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(snapshotRange.GetEnd() == Beam::Queries::Sequence(1));
  }

  TEST_CASE("date_constructors") {
    auto singleRange = Range(ptime(date(1984, May, 6)),
      ptime(date(1984, May, 6)));
    REQUIRE(singleRange.GetStart() == ptime(date(1984, May, 6)));
    REQUIRE(singleRange.GetEnd() == ptime(date(1984, May, 6)));
    auto openRange = Range(ptime(date(1984, May, 6)),
      ptime(date(2014, May, 6)));
    REQUIRE(openRange.GetStart() == ptime(date(1984, May, 6)));
    REQUIRE(openRange.GetEnd() == ptime(date(2014, May, 6)));
    auto snapshotRealTimeRange = Range(ptime(date(2014, May, 6)), pos_infin);
    REQUIRE(snapshotRealTimeRange.GetStart() == ptime(date(2014, May, 6)));
    REQUIRE(snapshotRealTimeRange.GetEnd() == Beam::Queries::Sequence::Last());
    auto snapshotRange = Range(neg_infin, ptime(date(2014, May, 6)));
    REQUIRE(snapshotRange.GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(snapshotRange.GetEnd() == ptime(date(2014, May, 6)));
  }

  TEST_CASE("mixed_constructors") {
    auto dateSequenceRange = Range(ptime(date(1984, May, 6)),
      Beam::Queries::Sequence(5));
    REQUIRE(dateSequenceRange.GetStart() == ptime(date(1984, May, 6)));
    REQUIRE(dateSequenceRange.GetEnd() == Beam::Queries::Sequence(5));
    auto sequenceDateRange = Range(Beam::Queries::Sequence(5),
      ptime(date(1984, May, 6)));
    REQUIRE(sequenceDateRange.GetStart() == Beam::Queries::Sequence(5));
    REQUIRE(sequenceDateRange.GetEnd() == ptime(date(1984, May, 6)));
    auto sequenceDateSnapshotRealTimeRange = Range(Beam::Queries::Sequence(5),
      pos_infin);
    REQUIRE(sequenceDateSnapshotRealTimeRange.GetStart() ==
      Beam::Queries::Sequence(5));
    REQUIRE(sequenceDateSnapshotRealTimeRange.GetEnd() ==
      Beam::Queries::Sequence::Last());
    auto dateSequenceSnapshotRealTimeRange = Range(ptime(date(1984, May, 6)),
      Beam::Queries::Sequence::Last());
    REQUIRE(dateSequenceSnapshotRealTimeRange.GetStart() ==
      ptime(date(1984, May, 6)));
    REQUIRE(dateSequenceSnapshotRealTimeRange.GetEnd() ==
      Beam::Queries::Sequence::Last());
    auto sequenceDateSnapshotRange = Range(Beam::Queries::Sequence::First(),
      ptime(date(1984, May, 6)));
    REQUIRE(sequenceDateSnapshotRange.GetStart() ==
      Beam::Queries::Sequence::First());
    REQUIRE(sequenceDateSnapshotRange.GetEnd() ==
      ptime(date(1984, May, 6)));
    auto dateSequenceSnapshotRange = Range(neg_infin,
      Beam::Queries::Sequence(5));
    REQUIRE(dateSequenceSnapshotRange.GetStart() ==
      Beam::Queries::Sequence::First());
    REQUIRE(dateSequenceSnapshotRange.GetEnd() == Beam::Queries::Sequence(5));
  }

  TEST_CASE("empty_range_value") {
    REQUIRE(Range::Empty().GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(Range::Empty().GetEnd() == Beam::Queries::Sequence::First());
  }

  TEST_CASE("total_range_value") {
    REQUIRE(Range::Total().GetStart() == Beam::Queries::Sequence::First());
    REQUIRE(Range::Total().GetEnd() == Beam::Queries::Sequence::Last());
  }

  TEST_CASE("real_time_range_value") {
    REQUIRE(Range::RealTime().GetStart() == Beam::Queries::Sequence::Present());
    REQUIRE(Range::RealTime().GetEnd() == Beam::Queries::Sequence::Last());
  }

  TEST_CASE("equals_operator") {
    REQUIRE(Range(Beam::Queries::Sequence(1), Beam::Queries::Sequence(2)) ==
      Range(Beam::Queries::Sequence(1), Beam::Queries::Sequence(2)));
    REQUIRE(!(Range(Beam::Queries::Sequence(1), Beam::Queries::Sequence(2)) ==
      Range(Beam::Queries::Sequence(2), Beam::Queries::Sequence(1))));
    REQUIRE(Range(Beam::Queries::Sequence::First(), Beam::Queries::Sequence(1)) ==
      Range(neg_infin, Beam::Queries::Sequence(1)));
    REQUIRE(Range(ptime(date(1984, May, 6)), Beam::Queries::Sequence(2)) ==
      Range(ptime(date(1984, May, 6)), Beam::Queries::Sequence(2)));
    REQUIRE(!(Range(ptime(date(1984, May, 6)), Beam::Queries::Sequence(2)) ==
      Range(Beam::Queries::Sequence(2), ptime(date(1984, May, 6)))));
    REQUIRE(Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))) ==
      Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))));
    REQUIRE(!(Range(ptime(date(1984, May, 6)),
      ptime(date(1985, May, 6))) == Range(ptime(date(1985, May, 6)),
      ptime(date(1985, May, 6)))));
  }

  TEST_CASE("not_equals_operator") {
    REQUIRE(!(Range(Beam::Queries::Sequence(1), Beam::Queries::Sequence(2)) !=
      Range(Beam::Queries::Sequence(1), Beam::Queries::Sequence(2))));
    REQUIRE(!(Range(Beam::Queries::Sequence::First(), Beam::Queries::Sequence(1)) !=
      Range(neg_infin, Beam::Queries::Sequence(1))));
    REQUIRE(!(Range(ptime(date(1984, May, 6)), Beam::Queries::Sequence(2)) !=
      Range(ptime(date(1984, May, 6)), Beam::Queries::Sequence(2))));
    REQUIRE(Range(ptime(date(1984, May, 6)), Beam::Queries::Sequence(2)) !=
      Range(Beam::Queries::Sequence(2), ptime(date(1984, May, 6))));
    REQUIRE(!(Range(ptime(date(1984, May, 6)),
      ptime(date(1985, May, 6))) != Range(ptime(date(1984, May, 6)),
      ptime(date(1985, May, 6)))));
    REQUIRE(Range(ptime(date(1984, May, 6)),
      ptime(date(1985, May, 6))) != Range(ptime(date(1985, May, 6)),
      ptime(date(1985, May, 6))));
  }
}
