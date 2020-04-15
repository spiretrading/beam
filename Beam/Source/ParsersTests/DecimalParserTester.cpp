#include <doctest/doctest.h>
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::Parsers;

TEST_SUITE("DecimalParser") {
  TEST_CASE("positive_decimal") {
    auto parser = DecimalParser<double>();
    auto source = ParserStreamFromString("3.300000");
    auto value = double();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == 3.3);
    source = ParserStreamFromString("1.a32");
    REQUIRE(!parser.Read(source, value));
  }

  TEST_CASE("negative_decimal") {
    auto parser = DecimalParser<double>();
    auto source = ParserStreamFromString("-3.1415");
    auto value = double();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == -3.1415);
    source = ParserStreamFromString("-a3.123");
    REQUIRE(!parser.Read(source, value));
  }
}
