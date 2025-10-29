#include <doctest/doctest.h>
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/TimeDurationParser.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("TimeDurationParser") {
  TEST_CASE("read_time_duration") {
    auto source = to_parser_stream("1:2:3.4");
    auto value = time_duration();
    REQUIRE(time_duration_p.read(source, value));
    REQUIRE(value == hours(1) + minutes(2) + seconds(3) + milliseconds(400));
    source = to_parser_stream("a:1:b");
    REQUIRE(!time_duration_p.read(source, value));
  }
}
