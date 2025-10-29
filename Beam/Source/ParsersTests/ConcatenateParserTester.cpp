#include <doctest/doctest.h>
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/EpsilonParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("ConcatenateParser") {
  TEST_CASE("void_parsers") {
    auto parser = eps_p >> 'a' >> 'b';
    auto source = to_parser_stream("");
    auto result = parser.read(source);
    REQUIRE(!result);
    source = to_parser_stream("a");
    result = parser.read(source);
    REQUIRE(!result);
    source = to_parser_stream("ab");
    result = parser.read(source);
    REQUIRE(result);
    source = to_parser_stream("dabc");
    result = parser.read(source);
    REQUIRE(!result);
  }

  TEST_CASE("left_void_parsers") {
    auto parser = 'a' >> int_p;
    auto source = to_parser_stream("");
    auto value = int();
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a");
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a5");
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 5);
  }

  TEST_CASE("right_void_parsers") {
    auto parser = int_p >> 'a';
    auto source = to_parser_stream("");
    auto value = int();
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a");
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("2a");
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 2);
  }

  TEST_CASE("no_void_parsers") {
    auto parser = int_p >> any_p;
    auto source = to_parser_stream("");
    auto value = std::tuple<int, char>();
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a");
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("24a");
    REQUIRE(parser.read(source, value));
    REQUIRE(std::get<0>(value) == 24);
    REQUIRE(std::get<1>(value) == 'a');
  }
}
