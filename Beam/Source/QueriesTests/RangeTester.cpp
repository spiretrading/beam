#include <sstream>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <doctest/doctest.h>
#include "Beam/Queries/Range.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("Range") {
  TEST_CASE("default_constructor") {
    auto range = Range();
    REQUIRE(range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(range.get_end() == Beam::Sequence::FIRST);
  }

  TEST_CASE("empty_range_constructors") {
    auto negative_infinity_range = Range(neg_infin, neg_infin);
    REQUIRE(negative_infinity_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(negative_infinity_range.get_end() == Beam::Sequence::FIRST);
    auto invalid_start_date_range = Range(not_a_date_time, Beam::Sequence(5));
    REQUIRE(invalid_start_date_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(invalid_start_date_range.get_end() == Beam::Sequence::FIRST);
    auto invalid_end_date_range = Range(Beam::Sequence(5), not_a_date_time);
    REQUIRE(invalid_end_date_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(invalid_end_date_range.get_end() == Beam::Sequence::FIRST);
    auto invalid_date_range = Range(not_a_date_time, not_a_date_time);
    REQUIRE(invalid_date_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(invalid_date_range.get_end() == Beam::Sequence::FIRST);
  }

  TEST_CASE("total_range_constructors") {
    auto total_date_range = Range(neg_infin, pos_infin);
    REQUIRE(total_date_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(total_date_range.get_end() == Beam::Sequence::LAST);
    auto total_sequence_range =
      Range(Beam::Sequence::FIRST, Beam::Sequence::LAST);
    REQUIRE(total_sequence_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(total_sequence_range.get_end() == Beam::Sequence::LAST);
    auto total_date_sequence_range = Range(neg_infin, Beam::Sequence::LAST);
    REQUIRE(total_date_sequence_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(total_date_sequence_range.get_end() == Beam::Sequence::LAST);
    auto total_sequence_date_range = Range(Beam::Sequence::FIRST, pos_infin);
    REQUIRE(total_sequence_date_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(total_sequence_date_range.get_end() == Beam::Sequence::LAST);
  }

  TEST_CASE("out_of_order_constructors") {
    auto infinite_range = Range(pos_infin, neg_infin);
    REQUIRE(infinite_range.get_start() == Beam::Sequence::LAST);
    REQUIRE(infinite_range.get_end() == Beam::Sequence::FIRST);
    auto sequence_range = Range(Beam::Sequence(5), Beam::Sequence(4));
    REQUIRE(sequence_range.get_start() == Beam::Sequence(5));
    REQUIRE(sequence_range.get_end() == Beam::Sequence(4));
    auto out_of_order_date_range =
      Range(ptime(date(2000, Jan, 13)), ptime(date(1999, Jan, 13)));
    REQUIRE(out_of_order_date_range.get_start() == ptime(date(2000, Jan, 13)));
    REQUIRE(out_of_order_date_range.get_end() == ptime(date(1999, Jan, 13)));
    auto out_of_order_infinite_range =
      Range(pos_infin, ptime(date(1999, Jan, 13)));
    REQUIRE(out_of_order_infinite_range.get_start() == Beam::Sequence::LAST);
    REQUIRE(
      out_of_order_infinite_range.get_end() == ptime(date(1999, Jan, 13)));
    auto out_of_order_negative_range =
      Range(ptime(date(1999, Jan, 13)), neg_infin);
    REQUIRE(
      out_of_order_negative_range.get_start() == ptime(date(1999, Jan, 13)));
    REQUIRE(out_of_order_negative_range.get_end() == Beam::Sequence::FIRST);
  }

  TEST_CASE("sequence_constructors") {
    auto single_range = Range(Beam::Sequence(1), Beam::Sequence(1));
    REQUIRE(single_range.get_start() == Beam::Sequence(1));
    REQUIRE(single_range.get_end() == Beam::Sequence(1));
    auto open_range = Range(Beam::Sequence(1), Beam::Sequence(2));
    REQUIRE(open_range.get_start() == Beam::Sequence(1));
    REQUIRE(open_range.get_end() == Beam::Sequence(2));
    auto snapshot_real_time_range =
      Range(Beam::Sequence(1), Beam::Sequence::LAST);
    REQUIRE(snapshot_real_time_range.get_start() == Beam::Sequence(1));
    REQUIRE(snapshot_real_time_range.get_end() == Beam::Sequence::LAST);
    auto snapshot_range = Range(Beam::Sequence::FIRST, Beam::Sequence(1));
    REQUIRE(snapshot_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(snapshot_range.get_end() == Beam::Sequence(1));
  }

  TEST_CASE("date_constructors") {
    auto single_range =
      Range(ptime(date(1984, May, 6)), ptime(date(1984, May, 6)));
    REQUIRE(single_range.get_start() == ptime(date(1984, May, 6)));
    REQUIRE(single_range.get_end() == ptime(date(1984, May, 6)));
    auto open_range =
      Range(ptime(date(1984, May, 6)), ptime(date(2014, May, 6)));
    REQUIRE(open_range.get_start() == ptime(date(1984, May, 6)));
    REQUIRE(open_range.get_end() == ptime(date(2014, May, 6)));
    auto snapshot_real_time_range = Range(ptime(date(2014, May, 6)), pos_infin);
    REQUIRE(snapshot_real_time_range.get_start() == ptime(date(2014, May, 6)));
    REQUIRE(snapshot_real_time_range.get_end() == Beam::Sequence::LAST);
    auto snapshot_range = Range(neg_infin, ptime(date(2014, May, 6)));
    REQUIRE(snapshot_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(snapshot_range.get_end() == ptime(date(2014, May, 6)));
  }

  TEST_CASE("mixed_constructors") {
    auto date_sequence_range =
      Range(ptime(date(1984, May, 6)), Beam::Sequence(5));
    REQUIRE(date_sequence_range.get_start() == ptime(date(1984, May, 6)));
    REQUIRE(date_sequence_range.get_end() == Beam::Sequence(5));
    auto sequence_date_range =
      Range(Beam::Sequence(5), ptime(date(1984, May, 6)));
    REQUIRE(sequence_date_range.get_start() == Beam::Sequence(5));
    REQUIRE(sequence_date_range.get_end() == ptime(date(1984, May, 6)));
    auto sequence_date_snapshot_real_time_range =
      Range(Beam::Sequence(5), pos_infin);
    REQUIRE(
      sequence_date_snapshot_real_time_range.get_start() == Beam::Sequence(5));
    REQUIRE(
      sequence_date_snapshot_real_time_range.get_end() == Beam::Sequence::LAST);
    auto date_sequence_snapshot_real_time_range =
      Range(ptime(date(1984, May, 6)), Beam::Sequence::LAST);
    REQUIRE(date_sequence_snapshot_real_time_range.get_start() ==
      ptime(date(1984, May, 6)));
    REQUIRE(
      date_sequence_snapshot_real_time_range.get_end() == Beam::Sequence::LAST);
    auto sequence_date_snapshot_range =
      Range(Beam::Sequence::FIRST, ptime(date(1984, May, 6)));
    REQUIRE(sequence_date_snapshot_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(
      sequence_date_snapshot_range.get_end() == ptime(date(1984, May, 6)));
    auto date_sequence_snapshot_range = Range(neg_infin, Beam::Sequence(5));
    REQUIRE(date_sequence_snapshot_range.get_start() == Beam::Sequence::FIRST);
    REQUIRE(date_sequence_snapshot_range.get_end() == Beam::Sequence(5));
  }

  TEST_CASE("empty_range_value") {
    REQUIRE(Range::EMPTY.get_start() == Beam::Sequence::FIRST);
    REQUIRE(Range::EMPTY.get_end() == Beam::Sequence::FIRST);
  }

  TEST_CASE("total_range_value") {
    REQUIRE(Range::TOTAL.get_start() == Beam::Sequence::FIRST);
    REQUIRE(Range::TOTAL.get_end() == Beam::Sequence::LAST);
  }

  TEST_CASE("real_time_range_value") {
    REQUIRE(Range::REAL_TIME.get_start() == Beam::Sequence::PRESENT);
    REQUIRE(Range::REAL_TIME.get_end() == Beam::Sequence::LAST);
  }

  TEST_CASE("equals_operator") {
    REQUIRE(Range(Beam::Sequence(1), Beam::Sequence(2)) ==
      Range(Beam::Sequence(1), Beam::Sequence(2)));
    REQUIRE(!(Range(Beam::Sequence(1), Beam::Sequence(2)) ==
      Range(Beam::Sequence(2), Beam::Sequence(1))));
    REQUIRE(Range(Beam::Sequence::FIRST, Beam::Sequence(1)) ==
      Range(neg_infin, Beam::Sequence(1)));
    REQUIRE(Range(ptime(date(1984, May, 6)), Beam::Sequence(2)) ==
      Range(ptime(date(1984, May, 6)), Beam::Sequence(2)));
    REQUIRE(!(Range(ptime(date(1984, May, 6)), Beam::Sequence(2)) ==
      Range(Beam::Sequence(2), ptime(date(1984, May, 6)))));
    REQUIRE(Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))) ==
      Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))));
    REQUIRE(!(Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))) ==
      Range(ptime(date(1985, May, 6)), ptime(date(1985, May, 6)))));
  }

  TEST_CASE("not_equals_operator") {
    REQUIRE(!(Range(Beam::Sequence(1), Beam::Sequence(2)) !=
      Range(Beam::Sequence(1), Beam::Sequence(2))));
    REQUIRE(!(Range(Beam::Sequence::FIRST, Beam::Sequence(1)) !=
      Range(neg_infin, Beam::Sequence(1))));
    REQUIRE(!(Range(ptime(date(1984, May, 6)), Beam::Sequence(2)) !=
      Range(ptime(date(1984, May, 6)), Beam::Sequence(2))));
    REQUIRE(Range(ptime(date(1984, May, 6)), Beam::Sequence(2)) !=
      Range(Beam::Sequence(2), ptime(date(1984, May, 6))));
    REQUIRE(!(Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))) !=
      Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6)))));
    REQUIRE(Range(ptime(date(1984, May, 6)), ptime(date(1985, May, 6))) !=
      Range(ptime(date(1985, May, 6)), ptime(date(1985, May, 6))));
  }

  TEST_CASE("stream") {
    auto buffer = std::stringstream();
    buffer << Range::EMPTY;
    REQUIRE(buffer.str() == "Empty");
    buffer.str("");
    buffer << Range::TOTAL;
    REQUIRE(buffer.str() == "Total");
    buffer.str("");
    buffer << Range(ptime(date(1984, May, 6)), ptime(date(2014, May, 6)));
    REQUIRE(buffer.str() == "(1984-May-06 00:00:00 2014-May-06 00:00:00)");
    buffer.str("");
    buffer << Range(Beam::Sequence::FIRST, Beam::Sequence(5555));
    REQUIRE(buffer.str() == "(0 5555)");
    test_round_trip_shuttle(
      Range(ptime(date(1984, May, 6)), ptime(date(2014, May, 6))));
  }
}
