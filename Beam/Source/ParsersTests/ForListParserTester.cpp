#include <doctest/doctest.h>
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/ForListParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("ForListParser") {
  TEST_CASE("accumulate_sum") {
    auto parser = for_list(int_p, 0, ',', [] (auto& accumulator, auto& v) {
      accumulator += v;
    });
    auto source = to_parser_stream("1,2,3");
    auto value = int();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 6);
    auto next = int();
    REQUIRE(!int_p.read(source, next));
  }

  TEST_CASE("empty_list") {
    auto parser = for_list(int_p, 10, ',', [] (auto& accumulator, auto& v) {
      accumulator += v;
    });
    auto source = to_parser_stream("");
    auto value = int();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 10);
  }

  TEST_CASE("handles_spaces_between_elements") {
    auto parser = for_list(int_p, 0, ',', [] (auto& accumulator, auto& v) {
      accumulator += v;
    });
    auto source = to_parser_stream("1 ,  2 ,  3  ");
    auto value = int();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 6);
  }

  TEST_CASE("stops_before_bad_element_and_leaves_stream") {
    auto parser = for_list(int_p, 0, ',', [] (auto& accumulator, auto& v) {
      accumulator += v;
    });
    auto source = to_parser_stream("1,2,x,3");
    auto value = int();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 3);
    auto ch = char();
    REQUIRE(any_p.read(source, ch));
    REQUIRE(ch == ',');
  }
}
