#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/BoolParser.hpp"
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/DifferenceParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/OrParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/StarParser.hpp"
#include "Beam/Utilities/OverloadSet.hpp"

using namespace Beam;

TEST_SUITE("OrParser") {
  TEST_CASE("chaining_no_null_parsers_with_no_duplicate_types") {
    auto parser = ('{' >> *(any_p - '}') >> '}') | int_p | bool_p | alpha_p;
    auto source = to_parser_stream("");
    auto value = decltype(parser)::Result();
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a");
    REQUIRE(parser.read(source, value));
    REQUIRE(visit(value,
      [] (char value) { return value == 'a'; },
      [] (const auto&) { return false; }));
    source = to_parser_stream("{hello}");
    REQUIRE(parser.read(source, value));
    REQUIRE(visit(value,
      [] (const std::string& value) { return value == "hello"; },
      [] (const auto&) { return false; }));
    source = to_parser_stream("123");
    REQUIRE(parser.read(source, value));
    REQUIRE(visit(value,
      [] (int value) { return value == 123; },
      [] (const auto&) { return false; }));
    source = to_parser_stream("true");
    REQUIRE(parser.read(source, value));
    REQUIRE(visit(value,
      [] (bool value) { return value; },
      [] (const auto&) { return false; }));
  }

  TEST_CASE("chaining_no_null_parsers_with_duplicate_types") {
    auto parser = (int_p - '1') | bool_p | alpha_p | int_p;
    auto source = to_parser_stream("");
    auto value = decltype(parser)::Result();
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a");
    REQUIRE(parser.read(source, value));
    REQUIRE(visit(value,
      [] (char value) { return value == 'a'; },
      [] (const auto&) { return false; }));
    source = to_parser_stream("123");
    REQUIRE(parser.read(source, value));
    REQUIRE(visit(value,
      [] (int value) { return value == 123; },
      [] (const auto&) { return false; }));
    source = to_parser_stream("456");
    REQUIRE(parser.read(source, value));
    REQUIRE(visit(value,
      [] (int value) { return value == 456; },
      [] (const auto&) { return false; }));
  }
}
