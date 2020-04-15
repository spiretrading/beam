#include <doctest/doctest.h>
#include "Beam/Parsers/TimeDurationParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("TimeDurationParser") {
  TEST_CASE("read_time_duration") {
    auto parser = TimeDurationParser();
    auto source = ParserStreamFromString("1:2:3.4");
    auto value = time_duration();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == hours(1) + minutes(2) + seconds(3) + milliseconds(400));
    source = ParserStreamFromString("a:1:b");
    REQUIRE(!parser.Read(source, value));
  }
}
