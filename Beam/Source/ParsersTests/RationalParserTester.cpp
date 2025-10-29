#include <doctest/doctest.h>
#include "Beam/Parsers/RationalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("RationalParser") {
  TEST_CASE("whole_numbers") {
    auto parser = RationalParser<int>();
    auto source = to_parser_stream("5");
    auto value = rational<int>();
    REQUIRE(parser.read(source, value));
    REQUIRE(value.numerator() == 5);
    REQUIRE(value.denominator() == 1);
    source = to_parser_stream("-2");
    REQUIRE(parser.read(source, value));
    REQUIRE(value.numerator() == -2);
    REQUIRE(value.denominator() == 1);
    source = to_parser_stream("-a5");
    REQUIRE(!parser.read(source, value));
  }

  TEST_CASE("within_zero_and_one") {
    auto parser = RationalParser<int>();
    auto source = to_parser_stream("0.5");
    auto value = rational<int>();
    REQUIRE(parser.read(source, value));
    REQUIRE(value.numerator() == 1);
    REQUIRE(value.denominator() == 2);
    source = to_parser_stream("-0.5");
    REQUIRE(parser.read(source, value));
    REQUIRE(value.numerator() == -1);
    REQUIRE(value.denominator() == 2);
    source = to_parser_stream("-.a5");
    REQUIRE(!parser.read(source, value));
  }
}
