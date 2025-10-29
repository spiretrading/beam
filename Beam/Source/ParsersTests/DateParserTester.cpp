#include <doctest/doctest.h>
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("DateParser") {
  TEST_CASE("read_date") {
    auto parser = date_parser();
    auto source = to_parser_stream("2003-02-11");
    auto value = date();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == date(2003, 2, 11));
    source = to_parser_stream("2003-02-ab");
    REQUIRE(!parser.read(source, value));
  }
}
