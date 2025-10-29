#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/SymbolParser.hpp"
#include "Beam/Parsers/Tokenize.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("Tokenize") {
  TEST_CASE("empty") {
    auto parser = tokenize();
    auto stream = to_parser_stream(from<SharedBuffer>("   "));
    REQUIRE(parser.read(stream));
  }

  TEST_CASE("symbol_and_int") {
    auto parser = tokenize("ID", int_p);
    auto stream = to_parser_stream(from<SharedBuffer>("  ID  123  "));
    auto value = int();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 123);
  }

  TEST_CASE("alpha_and_symbol") {
    auto parser = tokenize(alpha_p, "X");
    auto stream = to_parser_stream(from<SharedBuffer>("  a X "));
    auto value = char();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 'a');
  }

  TEST_CASE("alpha_int_and_symbol") {
    auto parser = tokenize(alpha_p, int_p, "Z");
    auto stream = to_parser_stream(from<SharedBuffer>("b 42 Z"));
    auto value = std::tuple<char, int>();
    REQUIRE(parser.read(stream, value));
    REQUIRE(std::get<0>(value) == 'b');
    REQUIRE(std::get<1>(value) == 42);
  }
}
