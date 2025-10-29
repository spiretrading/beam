#include <doctest/doctest.h>
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/DiscardParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ListParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("ListParser") {
  TEST_CASE("single_int_list") {
    auto parser = list(int_p, ',') >> discard(" .");
    auto source = to_parser_stream("123 .");
    auto values = std::vector<int>();
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 1);
    REQUIRE(values.front() == 123);
  }

  TEST_CASE("two_int_list") {
    auto parser = list(int_p, ',') >> discard(" .");
    auto source = to_parser_stream("123,456 .");
    auto values = std::vector<int>();
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 123);
    REQUIRE(values[1] == 456);
    source = to_parser_stream("321   ,654 .");
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 321);
    REQUIRE(values[1] == 654);
    source = to_parser_stream("98,   89 .");
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 98);
    REQUIRE(values[1] == 89);
    source = to_parser_stream("14  ,   52 .");
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 14);
    REQUIRE(values[1] == 52);
  }

  TEST_CASE("three_int_list") {
    auto parser = list(int_p, ',') >> discard(" .");
    auto source = to_parser_stream("123,456,789 .");
    auto values = std::vector<int>();
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 123);
    REQUIRE(values[1] == 456);
    REQUIRE(values[2] == 789);
    source = to_parser_stream("321,654  ,987 .");
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 321);
    REQUIRE(values[1] == 654);
    REQUIRE(values[2] == 987);
    source = to_parser_stream("98,89,   12 .");
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 98);
    REQUIRE(values[1] == 89);
    REQUIRE(values[2] == 12);
    source = to_parser_stream("14,52  ,   42 .");
    REQUIRE(parser.read(source, values));
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 14);
    REQUIRE(values[1] == 52);
    REQUIRE(values[2] == 42);
  }
}
