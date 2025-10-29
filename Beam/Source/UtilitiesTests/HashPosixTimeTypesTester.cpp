#include <unordered_map>
#include <unordered_set>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/Utilities/HashPosixTimeTypes.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

TEST_SUITE("HashPosixTimeTypes") {
  TEST_CASE("hash_ptime_same_value") {
    auto time1 = time_from_string("2024-01-15 10:30:00");
    auto time2 = time_from_string("2024-01-15 10:30:00");
    REQUIRE(hash_value(time1) == hash_value(time2));
  }

  TEST_CASE("hash_ptime_different_values") {
    auto time1 = time_from_string("2024-01-15 10:00:00");
    auto time2 = time_from_string("2024-01-15 11:00:00");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_ptime_before_epoch") {
    auto time = time_from_string("1969-12-31 23:00:00");
    auto hash = hash_value(time);
    REQUIRE(hash != 0);
  }

  TEST_CASE("hash_ptime_after_epoch") {
    auto time = time_from_string("2024-01-01 00:00:00");
    auto hash = hash_value(time);
    REQUIRE(hash != 0);
  }

  TEST_CASE("hash_ptime_microseconds") {
    auto time1 = time_from_string("2024-01-01 00:00:00.001000");
    auto time2 = time_from_string("2024-01-01 00:00:00.001001");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_ptime_use_in_std_hash") {
    auto time = time_from_string("2024-01-15 10:00:00");
    auto hasher = std::hash<ptime>();
    auto hash1 = hasher(time);
    auto hash2 = hasher(time);
    REQUIRE(hash1 == hash2);
  }

  TEST_CASE("hash_ptime_in_unordered_set") {
    auto set = std::unordered_set<ptime>();
    auto time1 = time_from_string("2024-01-01 10:00:00");
    auto time2 = time_from_string("2024-01-02 10:00:00");
    set.insert(time1);
    set.insert(time2);
    set.insert(time1);
    REQUIRE(set.size() == 2);
    REQUIRE(set.count(time1) == 1);
    REQUIRE(set.count(time2) == 1);
  }

  TEST_CASE("hash_ptime_in_unordered_map") {
    auto map = std::unordered_map<ptime, int>();
    auto time1 = time_from_string("2024-01-01 10:00:00");
    auto time2 = time_from_string("2024-01-02 10:00:00");
    map[time1] = 100;
    map[time2] = 200;
    REQUIRE(map[time1] == 100);
    REQUIRE(map[time2] == 200);
    REQUIRE(map.size() == 2);
  }

  TEST_CASE("hash_time_duration_same_value") {
    auto duration1 = duration_from_string("05:30:15");
    auto duration2 = duration_from_string("05:30:15");
    REQUIRE(hash_value(duration1) == hash_value(duration2));
  }

  TEST_CASE("hash_time_duration_different_values") {
    auto duration1 = duration_from_string("05:00:00");
    auto duration2 = duration_from_string("06:00:00");
    REQUIRE(hash_value(duration1) != hash_value(duration2));
  }

  TEST_CASE("hash_time_duration_negative") {
    auto duration = duration_from_string("-05:00:00");
    auto hash = hash_value(duration);
    REQUIRE(hash != 0);
  }

  TEST_CASE("hash_time_duration_microseconds") {
    auto duration1 = duration_from_string("00:00:00.001000");
    auto duration2 = duration_from_string("00:00:00.001001");
    REQUIRE(hash_value(duration1) != hash_value(duration2));
  }

  TEST_CASE("hash_time_duration_use_in_std_hash") {
    auto duration = duration_from_string("10:30:00");
    auto hasher = std::hash<time_duration>();
    auto hash1 = hasher(duration);
    auto hash2 = hasher(duration);
    REQUIRE(hash1 == hash2);
  }

  TEST_CASE("hash_time_duration_in_unordered_set") {
    auto set = std::unordered_set<time_duration>();
    auto duration1 = duration_from_string("01:00:00");
    auto duration2 = duration_from_string("02:00:00");
    set.insert(duration1);
    set.insert(duration2);
    set.insert(duration1);
    REQUIRE(set.size() == 2);
    REQUIRE(set.count(duration1) == 1);
    REQUIRE(set.count(duration2) == 1);
  }

  TEST_CASE("hash_time_duration_in_unordered_map") {
    auto map = std::unordered_map<time_duration, std::string>();
    auto duration1 = duration_from_string("01:00:00");
    auto duration2 = duration_from_string("02:00:00");
    map[duration1] = "one hour";
    map[duration2] = "two hours";
    REQUIRE(map[duration1] == "one hour");
    REQUIRE(map[duration2] == "two hours");
    REQUIRE(map.size() == 2);
  }

  TEST_CASE("hash_ptime_consistency") {
    auto time = time_from_string("2024-06-15 14:30:00");
    auto hash1 = hash_value(time);
    auto hash2 = hash_value(time);
    auto hash3 = hash_value(time);
    REQUIRE(hash1 == hash2);
    REQUIRE(hash2 == hash3);
  }

  TEST_CASE("hash_time_duration_consistency") {
    auto duration = duration_from_string("03:45:30");
    auto hash1 = hash_value(duration);
    auto hash2 = hash_value(duration);
    auto hash3 = hash_value(duration);
    REQUIRE(hash1 == hash2);
    REQUIRE(hash2 == hash3);
  }

  TEST_CASE("hash_ptime_close_values_different_hashes") {
    auto time1 = time_from_string("2024-01-01 00:00:00.000001");
    auto time2 = time_from_string("2024-01-01 00:00:00.000002");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_time_duration_seconds_vs_milliseconds") {
    auto duration1 = duration_from_string("00:00:01");
    auto duration2 = duration_from_string("00:00:01.000000");
    REQUIRE(hash_value(duration1) == hash_value(duration2));
  }

  TEST_CASE("hash_ptime_different_dates_same_time") {
    auto time1 = time_from_string("2024-01-01 10:00:00");
    auto time2 = time_from_string("2024-01-02 10:00:00");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_ptime_same_date_different_times") {
    auto time1 = time_from_string("2024-01-01 10:00:00");
    auto time2 = time_from_string("2024-01-01 11:00:00");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_time_duration_large_values") {
    auto duration1 = duration_from_string("10000:00:00");
    auto duration2 = duration_from_string("10001:00:00");
    REQUIRE(hash_value(duration1) != hash_value(duration2));
  }

  TEST_CASE("hash_ptime_year_boundaries") {
    auto time1 = time_from_string("2023-12-31 23:59:00");
    auto time2 = time_from_string("2024-01-01 00:00:00");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_ptime_leap_year") {
    auto time1 = time_from_string("2024-02-29 12:00:00");
    auto time2 = time_from_string("2024-03-01 12:00:00");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_time_duration_combined_in_map") {
    auto map = std::unordered_map<time_duration, ptime>();
    auto duration = duration_from_string("01:00:00");
    auto time = time_from_string("2024-01-01 10:00:00");
    map[duration] = time;
    REQUIRE(map[duration] == time);
  }

  TEST_CASE("hash_ptime_and_duration_independence") {
    auto time = time_from_string("2024-01-01 10:00:00");
    auto duration = duration_from_string("10:00:00");
    REQUIRE(hash_value(time) != hash_value(duration));
  }

  TEST_CASE("hash_ptime_max_precision") {
    auto time1 = time_from_string("2024-01-01 00:00:00.000001");
    auto time2 = time_from_string("2024-01-01 00:00:00.000002");
    REQUIRE(hash_value(time1) != hash_value(time2));
  }

  TEST_CASE("hash_time_duration_max_precision") {
    auto duration1 = duration_from_string("00:00:00.000001");
    auto duration2 = duration_from_string("00:00:00.000002");
    REQUIRE(hash_value(duration1) != hash_value(duration2));
  }
}
