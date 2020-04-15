#include <doctest/doctest.h>
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("DateParser") {
  TEST_CASE("read_date") {
    auto parser = DateParser();
    auto source = ParserStreamFromString("2003-02-11");
    auto value = date();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == date(2003, 2, 11));
    source = ParserStreamFromString("2003-02-ab");
    REQUIRE(!parser.Read(source, value));
  }
}
