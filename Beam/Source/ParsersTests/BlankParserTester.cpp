#include <doctest/doctest.h>
#include "Beam/Parsers/BlankParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("BlankParser") {
  TEST_CASE("parse_space_with_value") {
    auto stream = to_parser_stream(" a");
    auto parser = BlankParser();
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == ' ');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'a');
  }

  TEST_CASE("parse_tab_with_value") {
    auto source = std::string("\tB");
    auto stream = to_parser_stream(source);
    auto parser = BlankParser();
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == '\t');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'B');
  }

  TEST_CASE("non_blank") {
    auto stream = to_parser_stream("1");
    auto parser = BlankParser();
    auto value = char();
    REQUIRE(!parser.read(stream, value));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == '1');
  }

  TEST_CASE("empty_stream") {
    auto stream = to_parser_stream("");
    auto parser = BlankParser();
    auto value = char();
    REQUIRE(!parser.read(stream));
    REQUIRE(!parser.read(stream, value));
  }

  TEST_CASE("multiple_blanks") {
    auto stream = to_parser_stream("  a");
    auto parser = BlankParser();
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == ' ');
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == ' ');
    REQUIRE(!parser.read(stream));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'a');
  }
}
