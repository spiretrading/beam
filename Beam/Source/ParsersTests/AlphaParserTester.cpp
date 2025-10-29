#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("AlphaParser") {
  TEST_CASE("parse_alpha_with_value") {
    auto stream = to_parser_stream("x");
    auto parser = AlphaParser();
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 'x');
    REQUIRE(!stream.read());
  }

  TEST_CASE("parse_alpha_without_value") {
    auto stream = to_parser_stream("y");
    auto parser = AlphaParser();
    REQUIRE(parser.read(stream));
    REQUIRE(!stream.read());
  }

  TEST_CASE("parse_non_alpha") {
    auto stream = to_parser_stream("1");
    auto parser = AlphaParser();
    auto value = char();
    REQUIRE(!parser.read(stream, value));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == '1');
  }

  TEST_CASE("multiple_alphas_then_non_alpha") {
    auto stream = to_parser_stream("ab1");
    auto parser = AlphaParser();
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 'a');
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 'b');
    REQUIRE(!parser.read(stream));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == '1');
  }

  TEST_CASE("empty_stream") {
    auto stream = to_parser_stream("");
    auto parser = AlphaParser();
    auto value = char();
    REQUIRE(!parser.read(stream));
    REQUIRE(!parser.read(stream, value));
  }
}
