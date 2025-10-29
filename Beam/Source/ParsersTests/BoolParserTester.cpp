#include <doctest/doctest.h>
#include "Beam/Parsers/BoolParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("BoolParser") {
  TEST_CASE("parse_true_with_value") {
    auto stream = to_parser_stream("true");
    auto parser = BoolParser();
    auto value = bool();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == true);
    REQUIRE(!stream.read());
  }

  TEST_CASE("parse_false_with_value") {
    auto stream = to_parser_stream("false");
    auto parser = BoolParser();
    auto value = bool();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == false);
    REQUIRE(!stream.read());
  }

  TEST_CASE("parse_true_without_value") {
    auto stream = to_parser_stream("true");
    auto parser = BoolParser();
    REQUIRE(parser.read(stream));
    REQUIRE(!stream.read());
  }

  TEST_CASE("partial_mismatch") {
    auto stream = to_parser_stream("truX");
    auto parser = BoolParser();
    REQUIRE(!parser.read(stream));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 't');
  }

  TEST_CASE("mixed_token_followed_by_other_char") {
    auto stream = to_parser_stream("truex");
    auto parser = BoolParser();
    REQUIRE(parser.read(stream));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'x');
    REQUIRE(!stream.read());
  }

  TEST_CASE("case_sensitivity") {
    auto stream = to_parser_stream("True");
    auto parser = BoolParser();
    REQUIRE(!parser.read(stream));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'T');
  }

  TEST_CASE("empty_stream") {
    auto stream = to_parser_stream("");
    auto parser = BoolParser();
    auto value = bool();
    REQUIRE(!parser.read(stream));
    REQUIRE(!parser.read(stream, value));
  }
}
