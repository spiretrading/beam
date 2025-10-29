#include <doctest/doctest.h>
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("DecimalParser") {
  TEST_CASE("positive_decimal") {
    auto parser = DecimalParser<double>();
    auto source = to_parser_stream("3.300000");
    auto value = double();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 3.3);
    source = to_parser_stream("1.a32");
    REQUIRE(!parser.read(source, value));
  }

  TEST_CASE("negative_decimal") {
    auto parser = DecimalParser<double>();
    auto source = to_parser_stream("-3.1415");
    auto value = double();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == -3.1415);
    source = to_parser_stream("-a3.123");
    REQUIRE(!parser.read(source, value));
  }
}
