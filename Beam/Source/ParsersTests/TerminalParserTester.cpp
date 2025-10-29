#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/TerminalParser.hpp"

using namespace Beam;

TEST_SUITE("TerminalParser") {
  TEST_CASE("match_exact_character") {
    auto parser = TerminalParser('x');
    auto source = to_parser_stream("xA");
    REQUIRE(parser.read(source));
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'A');
  }

  TEST_CASE("fail_on_different_character_and_stream_unchanged") {
    auto parser = TerminalParser('x');
    auto source = to_parser_stream("y");
    REQUIRE(!parser.read(source));
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'y');
  }

  TEST_CASE("eof_fails") {
    auto parser = TerminalParser('z');
    auto source = to_parser_stream("");
    REQUIRE(!parser.read(source));
  }

  TEST_CASE("multiple_terminals_consume_in_order") {
    auto p1 = TerminalParser('a');
    auto p2 = TerminalParser('b');
    auto source = to_parser_stream("abC");
    REQUIRE(p1.read(source));
    REQUIRE(p2.read(source));
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'C');
  }

  TEST_CASE("failure_restores_stream_for_subsequent_parses") {
    auto p = TerminalParser('1');
    auto source = to_parser_stream("2x");
    REQUIRE(!p.read(source));
    auto d = char();
    REQUIRE(digit_p.read(source, d));
    REQUIRE(d == '2');
  }
}
