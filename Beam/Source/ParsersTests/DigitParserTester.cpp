#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("DigitParser") {
  TEST_CASE("match_digit_with_value") {
    auto source = to_parser_stream("7");
    auto value = char();
    REQUIRE(digit_p.read(source, value));
    REQUIRE(value == '7');
  }

  TEST_CASE("match_digit_without_value") {
    auto source = to_parser_stream("5x");
    REQUIRE(digit_p.read(source));
    auto next = char();
    REQUIRE(alpha_p.read(source, next));
    REQUIRE(next == 'x');
  }

  TEST_CASE("multiple_digits_sequence") {
    auto source = to_parser_stream("123");
    auto value = char();
    REQUIRE(digit_p.read(source, value));
    REQUIRE(value == '1');
    REQUIRE(digit_p.read(source, value));
    REQUIRE(value == '2');
    REQUIRE(digit_p.read(source, value));
    REQUIRE(value == '3');
    REQUIRE(!digit_p.read(source, value));
  }

  TEST_CASE("non_digit") {
    auto source = to_parser_stream("a1");
    auto initial = 'Z';
    auto value = initial;
    REQUIRE(!digit_p.read(source, value));
    auto read_char = char();
    REQUIRE(alpha_p.read(source, read_char));
    REQUIRE(read_char == 'a');
  }

  TEST_CASE("non_digit_without_value") {
    auto source = to_parser_stream("z9");
    REQUIRE(!digit_p.read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'z');
  }
}
