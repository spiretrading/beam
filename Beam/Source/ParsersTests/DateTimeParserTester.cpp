#include <doctest/doctest.h>
#include "Beam/Parsers/DateTimeParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("DateTimeParser") {
  TEST_CASE("read_date_time") {
    auto parser = DateTimeParser();
    auto source = ParserStreamFromString("2003-02-11 13:04:53.1");
    auto value = ptime();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value.date() == date(2003, 2, 11));
    REQUIRE(value.time_of_day() == hours(13) + minutes(4) + seconds(53) +
      milliseconds(100));
    source = ParserStreamFromString("2003-02-11");
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("13:04:53.1");
    REQUIRE(!parser.Read(source, value));
  }
}
