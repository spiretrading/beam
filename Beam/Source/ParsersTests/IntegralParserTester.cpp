#include <doctest/doctest.h>
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("IntegralParser") {
  TEST_CASE("positive_int") {
    auto parser = IntegralParser<int>();
    auto source = to_parser_stream("123");
    auto value = int();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 123);
    source = to_parser_stream("a123");
    REQUIRE(!parser.read(source, value));
  }

  TEST_CASE("negative_int") {
    auto parser = IntegralParser<int>();
    auto source = to_parser_stream("-123");
    auto value = int();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == -123);
    source = to_parser_stream("-a123");
    REQUIRE(!parser.read(source, value));
  }
}
