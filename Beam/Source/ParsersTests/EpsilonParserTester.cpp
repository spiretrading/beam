#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/EpsilonParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("EpsilonParser") {
  TEST_CASE("empty") {
    auto source = to_parser_stream("");
    REQUIRE(eps_p.read(source));
  }

  TEST_CASE("consume_input") {
    auto source = to_parser_stream("hello");
    REQUIRE(eps_p.read(source));
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'h');
  }

  TEST_CASE("idempotent_calls") {
    auto source = to_parser_stream("z");
    REQUIRE(eps_p.read(source));
    REQUIRE(eps_p.read(source));
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'z');
  }
}
