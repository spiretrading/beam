#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/StarParser.hpp"

using namespace Beam;

TEST_SUITE("StarParser") {
  TEST_CASE("char_specialization_collects_characters") {
    auto parser = star(alpha_p);
    auto source = to_parser_stream("abc1");
    auto value = std::string();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == "abc");
    auto next = char();
    REQUIRE(digit_p.read(source, next));
    REQUIRE(next == '1');
  }

  TEST_CASE("char_specialization_empty_input") {
    auto parser = star(alpha_p);
    auto source = to_parser_stream("");
    auto value = std::string();
    REQUIRE(parser.read(source, value));
    REQUIRE(value.empty());
    REQUIRE(!alpha_p.read(source));
  }

  TEST_CASE("char_specialization_read_without_value_consumes") {
    auto parser = star(alpha_p);
    auto source = to_parser_stream("xyz!");
    REQUIRE(parser.read(source));
    auto c = char();
    REQUIRE(symbol("!").read(source));
  }

  TEST_CASE("void_specialization_consumes_repeated_symbols") {
    auto parser = star(symbol("x"));
    auto source = to_parser_stream("xxxY");
    REQUIRE(parser.read(source));
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'Y');
  }

  TEST_CASE("void_specialization_empty_does_not_consume") {
    auto parser = star(symbol("x"));
    auto source = to_parser_stream("abc");
    REQUIRE(parser.read(source));
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'a');
  }

  TEST_CASE("non_char_specialization_collects_converted_digits") {
    auto to_int = convert(digit_p, [] (char c) {
      return static_cast<int>(c - '0');
    });
    auto parser = star(to_int);
    auto source = to_parser_stream("123x");
    auto value = std::vector<int>();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == std::vector{1, 2, 3});
    auto ch = char();
    REQUIRE(alpha_p.read(source, ch));
    REQUIRE(ch == 'x');
  }

  TEST_CASE("non_char_specialization_empty_input") {
    auto to_int = convert(digit_p, [] (char c) {
      return static_cast<int>(c - '0');
    });
    auto parser = star(to_int);
    auto source = to_parser_stream("z");
    auto value = std::vector<int>();
    REQUIRE(parser.read(source, value));
    REQUIRE(value.empty());
    auto c = char();
    REQUIRE(alpha_p.read(source, c));
    REQUIRE(c == 'z');
  }

  TEST_CASE("non_char_read_without_value_consumes_digits") {
    auto to_int = convert(digit_p, [] (char c) {
      return static_cast<int>(c - '0');
    });
    auto parser = star(to_int);
    auto source = to_parser_stream("42!");
    REQUIRE(parser.read(source));
    auto ex = symbol("!");
    REQUIRE(ex.read(source));
  }
}
