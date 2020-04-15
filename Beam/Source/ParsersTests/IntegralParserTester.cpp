#include <doctest/doctest.h>
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::Parsers;

TEST_SUITE("IntegralParser") {
  TEST_CASE("positive_int") {
    auto parser = IntegralParser<int>();
    auto source = ParserStreamFromString("123");
    auto value = int();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == 123);
    source = ParserStreamFromString("a123");
    REQUIRE(!parser.Read(source, value));
  }

  TEST_CASE("negative_int") {
    auto parser = IntegralParser<int>();
    auto source = ParserStreamFromString("-123");
    auto value = int();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == -123);
    source = ParserStreamFromString("-a123");
    REQUIRE(!parser.Read(source, value));
  }
}
