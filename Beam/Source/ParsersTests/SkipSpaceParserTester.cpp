#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"

using namespace Beam;

TEST_SUITE("SkipSpaceParser") {
  TEST_CASE("empty_input") {
    auto source = to_parser_stream("");
    REQUIRE(SkipSpaceParser().read(source));
    REQUIRE(!alpha_p.read(source));
  }

  TEST_CASE("only_spaces") {
    auto source = to_parser_stream("   \t\n  ");
    REQUIRE(SkipSpaceParser().read(source));
    REQUIRE(!alpha_p.read(source));
  }

  TEST_CASE("spaces_then_char") {
    auto source = to_parser_stream("   a");
    REQUIRE(SkipSpaceParser().read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'a');
  }

  TEST_CASE("various_whitespace_chars") {
    auto source = to_parser_stream("\t \n\r\tb");
    REQUIRE(SkipSpaceParser().read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'b');
  }

  TEST_CASE("non_space_first_char") {
    auto source = to_parser_stream("x ");
    REQUIRE(SkipSpaceParser().read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'x');
  }
}
