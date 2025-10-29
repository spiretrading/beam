#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/NoneParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("NoneParser") {
  TEST_CASE("read_with_value") {
    auto source = to_parser_stream("123");
    auto value = int(-1);
    REQUIRE(!none_p<int>.read(source, value));
    REQUIRE(value == -1);
    auto parsed = int();
    REQUIRE(int_p.read(source, parsed));
    REQUIRE(parsed == 123);
  }

  TEST_CASE("read_without_value") {
    auto source = to_parser_stream("456");
    REQUIRE(!none_p<int>.read(source));
    auto parsed = int();
    REQUIRE(int_p.read(source, parsed));
    REQUIRE(parsed == 456);
  }

  TEST_CASE("empty_stream") {
    auto source = to_parser_stream("");
    auto value = int(10);
    REQUIRE(!none_p<int>.read(source, value));
    REQUIRE(value == 10);
  }

  TEST_CASE("empty_stream_fails_without_value") {
    auto source = to_parser_stream("");
    REQUIRE(!none_p<int>.read(source));
  }

  TEST_CASE("no_side_effects_on_stream_char") {
    auto source = to_parser_stream("a1");
    auto value = char('Z');
    REQUIRE(!none_p<char>.read(source, value));
    REQUIRE(value == 'Z');
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'a');
  }

  TEST_CASE("no_side_effects_on_stream_char_without_value") {
    auto source = to_parser_stream("b2");
    REQUIRE(!none_p<>.read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'b');
  }
}
