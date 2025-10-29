#include <doctest/doctest.h>
#include "Beam/Sql/Conversions.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Viper;

TEST_SUITE("SqlConversions") {
  TEST_CASE("time_duration_round_trip") {
    auto duration = microseconds(123456);
    auto column = std::string();
    ToSql<time_duration>()(duration, column);
    auto raw = RawColumn(column.c_str(), column.size());
    auto result = FromSql<time_duration>()(raw);
    REQUIRE(result.total_microseconds() == duration.total_microseconds());
    auto negative = microseconds(-987654321);
    column.clear();
    ToSql<time_duration>()(negative, column);
    raw = RawColumn(column.c_str(), column.size());
    result = FromSql<time_duration>()(raw);
    REQUIRE(result.total_microseconds() == negative.total_microseconds());
  }

  TEST_CASE("sequence_round_trip") {
    auto sequence = Sequence(42);
    auto column = std::string();
    ToSql<Sequence>()(sequence, column);
    auto raw = RawColumn(column.c_str(), column.size());
    auto result = FromSql<Sequence>()(raw);
    REQUIRE(result.get_ordinal() == sequence.get_ordinal());
  }

  TEST_CASE("fixed_string_from_raw_column") {
    auto text = "hello";
    auto raw = RawColumn(text, std::strlen(text));
    auto result = FromSql<FixedString<16>>()(raw);
    REQUIRE(result == std::string(text));
  }
}
