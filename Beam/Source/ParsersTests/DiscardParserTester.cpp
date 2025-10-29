#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/DiscardParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("DiscardParser") {
  TEST_CASE("discard_space") {
    auto source = to_parser_stream("  a");
    REQUIRE(space_p.read(source));
    REQUIRE(space_p.read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'a');
  }

  TEST_CASE("discard_alpha_then_digit") {
    auto parser = discard(alpha_p);
    auto source = to_parser_stream("x1");
    REQUIRE(parser.read(source));
    auto d = char();
    REQUIRE(digit_p.read(source, d));
    REQUIRE(d == '1');
  }

  TEST_CASE("discard_fails") {
    auto parser = discard(alpha_p);
    auto source = to_parser_stream("1a");
    REQUIRE(!parser.read(source));
    auto d = char();
    REQUIRE(digit_p.read(source, d));
    REQUIRE(d == '1');
  }
}
