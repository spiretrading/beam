#include <doctest/doctest.h>
#include "Beam/Parsers/DifferenceParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/SymbolParser.hpp"

using namespace Beam;

TEST_SUITE("DifferenceParser") {
  TEST_CASE("left_matches_when_right_absent") {
    auto parser = int_p - "x";
    auto source = to_parser_stream("123");
    auto value = int();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 123);
    auto next = int();
    REQUIRE(!int_p.read(source, next));
  }

  TEST_CASE("right_matches_prevents_left") {
    auto parser = int_p - "1";
    auto source = to_parser_stream("123");
    auto value = int();
    REQUIRE(!parser.read(source, value));
    auto value2 = int();
    REQUIRE(int_p.read(source, value2));
    REQUIRE(value2 == 123);
  }

  TEST_CASE("right_precedence_over_left") {
    auto left = symbol("12");
    auto parser = left - "1";
    auto source = to_parser_stream("12");
    REQUIRE(!parser.read(source));
    REQUIRE(left.read(source));
  }

  TEST_CASE("no_match_when_left_does_not_match") {
    auto parser = int_p - "2";
    auto source = to_parser_stream("abc");
    auto value = int();
    REQUIRE(!parser.read(source, value));
    auto s = to_parser_stream("abc");
    REQUIRE(!int_p.read(s));
  }
}
