#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/SequenceParser.hpp"
#include "Beam/Parsers/SymbolParser.hpp"

using namespace Beam;

TEST_SUITE("SequenceParser") {
  TEST_CASE("parse_sequence_of_ints") {
    auto parser = sequence(std::vector{int_p, int_p, int_p}, ',');
    auto source = to_parser_stream("1,2,3");
    auto value = std::vector<int>();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == std::vector{1, 2, 3});
    auto next = int();
    REQUIRE(!int_p.read(source, next));
  }

  TEST_CASE("parse_sequence_of_chars_with_dash_delimiter") {
    auto parser = sequence(std::vector{alpha_p, alpha_p}, '-');
    auto source = to_parser_stream("a-b");
    auto value = std::vector<char>();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == std::vector{'a', 'b'});
  }

  TEST_CASE("void_sequence_of_symbols") {
    auto ptrue = symbol("true");
    auto pfalse = symbol("false");
    auto parser = sequence(std::vector{ptrue, pfalse}, ',');
    auto source = to_parser_stream("true,false");
    REQUIRE(parser.read(source));
    REQUIRE(!symbol("true").read(source));
  }

  TEST_CASE("stops_and_restores_stream_on_failure") {
    auto parser = sequence(std::vector{int_p, int_p, int_p}, ',');
    auto source = to_parser_stream("1,x,3");
    auto value = std::vector<int>();
    REQUIRE(!parser.read(source, value));
    auto first = int();
    REQUIRE(int_p.read(source, first));
    REQUIRE(first == 1);
  }

  TEST_CASE("handles_spaces_around_delimiters") {
    auto parser = sequence(std::vector{int_p, int_p, int_p}, ',');
    auto source = to_parser_stream("  1 ,  2 ,3  ");
    auto value = std::vector<int>();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == std::vector{1, 2, 3});
  }

  TEST_CASE("fails_when_missing_delimiter") {
    auto parser = sequence(std::vector{alpha_p, alpha_p}, ',');
    auto source = to_parser_stream("ab");
    auto value = std::vector<char>();
    REQUIRE(!parser.read(source, value));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'a');
  }
}
