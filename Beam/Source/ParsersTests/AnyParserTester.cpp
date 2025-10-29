#include <doctest/doctest.h>
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("AnyParser") {
  TEST_CASE("parse_any_with_value") {
    auto stream = to_parser_stream("x");
    auto parser = AnyParser();
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 'x');
    REQUIRE(!stream.read());
  }

  TEST_CASE("parse_any_without_value") {
    auto stream = to_parser_stream("y");
    auto parser = AnyParser();
    REQUIRE(parser.read(stream));
    REQUIRE(!stream.read());
  }

  TEST_CASE("multiple_characters") {
    auto stream = to_parser_stream("ab1");
    auto parser = AnyParser();
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 'a');
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 'b');
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == '1');
    REQUIRE(!stream.read());
  }

  TEST_CASE("empty_stream_returns_false") {
    auto stream = to_parser_stream("");
    auto parser = AnyParser();
    auto value = char();
    REQUIRE(!parser.read(stream));
    REQUIRE(!parser.read(stream, value));
  }

  TEST_CASE("read_long_string") {
    auto length = std::size_t(2000);
    auto source = std::string(length, 'Z');
    auto stream = to_parser_stream(source);
    auto parser = AnyParser();
    auto count = std::size_t(0);
    while(parser.read(stream)) {
      ++count;
    }
    REQUIRE(count == length);
  }
}
