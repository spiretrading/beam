#include <doctest/doctest.h>
#include "Beam/Parsers/ListParser.hpp"
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Types.hpp"

using namespace Beam;
using namespace Beam::Parsers;

TEST_SUITE("ListParser") {
  TEST_CASE("single_int_list") {
    auto parser = List(int_p, ',') >> Discard(" .");
    auto source = ParserStreamFromString("123 .");
    auto values = std::vector<int>();
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 1);
    REQUIRE(values.front() == 123);
  }

  TEST_CASE("two_int_list") {
    auto parser = List(int_p, ',') >> Discard(" .");
    auto source = ParserStreamFromString("123,456 .");
    auto values = std::vector<int>();
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 123);
    REQUIRE(values[1] == 456);
    source = ParserStreamFromString("321   ,654 .");
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 321);
    REQUIRE(values[1] == 654);
    source = ParserStreamFromString("98,   89 .");
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 98);
    REQUIRE(values[1] == 89);
    source = ParserStreamFromString("14  ,   52 .");
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 14);
    REQUIRE(values[1] == 52);
  }

  TEST_CASE("three_int_list") {
    auto parser = List(int_p, ',') >> Discard(" .");
    auto source = ParserStreamFromString("123,456,789 .");
    auto values = std::vector<int>();
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 123);
    REQUIRE(values[1] == 456);
    REQUIRE(values[2] == 789);
    source = ParserStreamFromString("321,654  ,987 .");
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 321);
    REQUIRE(values[1] == 654);
    REQUIRE(values[2] == 987);
    source = ParserStreamFromString("98,89,   12 .");
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 98);
    REQUIRE(values[1] == 89);
    REQUIRE(values[2] == 12);
    source = ParserStreamFromString("14,52  ,   42 .");
    REQUIRE(parser.Read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 14);
    REQUIRE(values[1] == 52);
    REQUIRE(values[2] == 42);
  }
}
