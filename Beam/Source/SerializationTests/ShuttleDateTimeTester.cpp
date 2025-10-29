#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleDateTime") {
  TEST_CASE("time_duration_basic") {
    test_round_trip_shuttle(time_duration(seconds(9015)));
  }

  TEST_CASE("time_duration_infinities") {
    test_round_trip_shuttle(time_duration(pos_infin));
    test_round_trip_shuttle(time_duration(neg_infin));
  }

  TEST_CASE("ptime_basic") {
    test_round_trip_shuttle(
      ptime(date(2020, 12, 31), hours(23) + minutes(59) + seconds(59)));
  }

  TEST_CASE("ptime_infinities_and_not_a_date_time") {
    test_round_trip_shuttle(ptime(pos_infin));
    test_round_trip_shuttle(ptime(neg_infin));
    test_round_trip_shuttle(ptime());
  }

  TEST_CASE("gregorian_weekday") {
    test_round_trip_shuttle(greg_weekday(3));
  }

  TEST_CASE("gregorian_day_month_year") {
    test_round_trip_shuttle(greg_day(15));
    test_round_trip_shuttle(greg_month(12));
    test_round_trip_shuttle(greg_year(2025));
  }

  TEST_CASE("date_basic_and_special") {
    test_round_trip_shuttle(date(2021, 1, 2));
    test_round_trip_shuttle(date(pos_infin));
    test_round_trip_shuttle(date(neg_infin));
    test_round_trip_shuttle(date());
  }
}
