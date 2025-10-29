#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/SpaceParser.hpp"

using namespace Beam;

TEST_SUITE("SpaceParser") {
  TEST_CASE("match_single_space_with_value") {
    auto source = to_parser_stream(" x");
    auto value = char();
    REQUIRE(SpaceParser().read(source, value));
    REQUIRE(value == ' ');
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'x');
  }

  TEST_CASE("match_various_whitespace_chars") {
    auto source = to_parser_stream("\tA");
    auto value = char();
    REQUIRE(SpaceParser().read(source, value));
    REQUIRE(value == '\t');
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'A');
  }

  TEST_CASE("multiple_spaces_consumed_sequentially") {
    auto source = to_parser_stream(" 7");
    auto v1 = char();
    REQUIRE(SpaceParser().read(source, v1));
    REQUIRE(v1 == ' ');
    auto d = char();
    REQUIRE(digit_p.read(source, d));
    REQUIRE(d == '7');
  }

  TEST_CASE("non_space_fails_and_stream_unchanged_with_value") {
    auto source = to_parser_stream("a ");
    auto value = char();
    REQUIRE(!SpaceParser().read(source, value));
    REQUIRE(alpha_p.read(source, value));
    REQUIRE(value == 'a');
  }

  TEST_CASE("non_space_fails_without_value_and_stream_unchanged") {
    auto source = to_parser_stream("b1");
    REQUIRE(!SpaceParser().read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'b');
  }

  TEST_CASE("eof_fails") {
    auto source = to_parser_stream("");
    REQUIRE(!SpaceParser().read(source));
    auto v = char();
    REQUIRE(!SpaceParser().read(source, v));
  }
}
