#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/SymbolParser.hpp"

using namespace Beam;

TEST_SUITE("SymbolParser") {
  TEST_CASE("match_exact_symbol") {
    auto parser = symbol("hello");
    auto source = to_parser_stream("hello");
    REQUIRE(parser.read(source));
    REQUIRE(!alpha_p.read(source));
  }

  TEST_CASE("partial_match_fails_and_stream_unchanged") {
    auto parser = symbol("hello");
    auto source = to_parser_stream("helx");
    REQUIRE(!parser.read(source));
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'h');
  }

  TEST_CASE("symbol_with_value_returns_value") {
    auto parser = symbol("yes", 99);
    {
      auto source = to_parser_stream("yes");
      auto v = int();
      REQUIRE(parser.read(source, v));
      REQUIRE(v == 99);
    }
    {
      auto source = to_parser_stream("no");
      auto v = int(-1);
      REQUIRE(!parser.read(source, v));
      REQUIRE(v == -1);
    }
  }

  TEST_CASE("empty_symbol_behaves_as_epsilon") {
    auto parser = symbol(std::string());
    auto source = to_parser_stream("abc");
    REQUIRE(parser.read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'a');
  }

  TEST_CASE("short_symbol_consumes_only_its_length") {
    auto pshort = symbol("a");
    auto source = to_parser_stream("ab");
    REQUIRE(pshort.read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'b');
  }
}
