#include <doctest/doctest.h>
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("PosixTimeToSqlDateTime") {
  TEST_CASE("convert_special_values") {
    auto value = std::numeric_limits<std::uint64_t>::max();
    REQUIRE(to_sql_timestamp(not_a_date_time) == value);
    REQUIRE(from_sql_timestamp(value) == not_a_date_time);
    REQUIRE(to_sql_timestamp(pos_infin) == (value - 1));
    REQUIRE(from_sql_timestamp(value - 1) == pos_infin);
    REQUIRE(to_sql_timestamp(neg_infin) == 0u);
    REQUIRE(from_sql_timestamp(0u) == neg_infin);
  }

  TEST_CASE("round_trip_conversion_for_non_epoch_time") {
    auto duration = 12345;
    auto time =
      ptime(gregorian::date(1970, gregorian::Jan, 1), milliseconds(duration));
    auto value = to_sql_timestamp(time);
    REQUIRE(value == duration);
    REQUIRE(from_sql_timestamp(value) == time);
  }

  TEST_CASE("epoch_behavior_and_ambiguity") {
    auto epoch = ptime(gregorian::date(1970, gregorian::Jan, 1), seconds(0));
    REQUIRE(to_sql_timestamp(epoch) == 0);
    REQUIRE(from_sql_timestamp(0) == neg_infin);
    REQUIRE(from_sql_timestamp(to_sql_timestamp(epoch)) != epoch);
  }
}
