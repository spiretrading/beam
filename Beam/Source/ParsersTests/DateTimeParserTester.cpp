#include <doctest/doctest.h>
#include "Beam/Parsers/DateTimeParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("DateTimeParser") {
  TEST_CASE("read_date_time") {
    auto parser = date_time_parser();
    auto source = to_parser_stream("2003-02-11 13:04:53.1");
    auto value = ptime();
    REQUIRE(parser.read(source, value));
    REQUIRE(value.date() == date(2003, 2, 11));
    REQUIRE(value.time_of_day() == hours(13) + minutes(4) + seconds(53) +
      milliseconds(100));
    source = to_parser_stream("2003-02-11");
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("13:04:53.1");
    REQUIRE(!parser.read(source, value));
  }
}
