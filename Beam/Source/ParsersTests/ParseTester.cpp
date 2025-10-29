#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Parsers/Parse.hpp"

using namespace Beam;

TEST_SUITE("Parse") {
  TEST_CASE("parse_with_parser_and_string") {
    auto result = parse(AnyParser(), "x");
    REQUIRE(result == 'x');
  }

  TEST_CASE("parse_with_parser_and_buffer") {
    auto buffer = from<SharedBuffer>("y");
    auto result = parse(AnyParser(), buffer);
    REQUIRE(result == 'y');
  }

  TEST_CASE("parse_with_default_parser_string") {
    auto result = parse<int>("123");
    REQUIRE(result == 123);
  }

  TEST_CASE("parse_with_default_parser_buffer") {
    auto buffer = from<SharedBuffer>("456");
    auto result = parse<int>(buffer);
    REQUIRE(result == 456);
  }

  TEST_CASE("parse_invalid_string_throws") {
    REQUIRE_THROWS_AS(parse(AlphaParser(), "1"), ParserException);
  }

  TEST_CASE("parse_invalid_buffer_throws") {
    auto buffer = from<SharedBuffer>("1");
    REQUIRE_THROWS_AS(parse(AlphaParser(), buffer), ParserException);
  }
}
