#include <doctest/doctest.h>
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Types.hpp"

using namespace Beam;
using namespace Beam::Parsers;

TEST_SUITE("ConcatenateParser") {
  TEST_CASE("void_parsers") {
    auto parser = eps_p >> 'a' >> 'b' >> 'c';
    auto source = ParserStreamFromString("");
    auto result = parser.Read(source);
    REQUIRE(!result);
    source = ParserStreamFromString("a");
    result = parser.Read(source);
    REQUIRE(!result);
    source = ParserStreamFromString("ab");
    result = parser.Read(source);
    REQUIRE(!result);
    source = ParserStreamFromString("abc");
    result = parser.Read(source);
    REQUIRE(result);
    source = ParserStreamFromString("abcd");
    result = parser.Read(source);
    REQUIRE(result);
    source = ParserStreamFromString("dabc");
    result = parser.Read(source);
    REQUIRE(!result);
  }

  TEST_CASE("left_void_parsers") {
    auto parser = 'a' >> int_p;
    auto source = ParserStreamFromString("");
    auto value = int();
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("a");
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("a5");
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == 5);
  }

  TEST_CASE("right_void_parsers") {
    auto parser = int_p >> 'a';
    auto source = ParserStreamFromString("");
    auto value = int();
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("a");
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("2a");
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == 2);
  }

  TEST_CASE("no_void_parsers") {
    auto parser = int_p >> any_p;
    auto source = ParserStreamFromString("");
    auto value = std::tuple<int, char>();
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("a");
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("24a");
    REQUIRE(parser.Read(source, value));
    REQUIRE(std::get<0>(value) == 24);
    REQUIRE(std::get<1>(value) == 'a');
  }
}
