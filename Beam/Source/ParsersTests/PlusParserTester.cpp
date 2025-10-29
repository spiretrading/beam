#include <doctest/doctest.h>
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/PlusParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("PlusParser") {
  TEST_CASE("plus_digit_parser") {
    auto stream = to_parser_stream("12345");
    auto parser = +digit_p;
    auto value = std::string();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == "12345");
    REQUIRE(!stream.read());
  }

  TEST_CASE("plus_of_digit_parser_empty") {
    auto stream = to_parser_stream("");
    auto parser = +digit_p;
    REQUIRE(!parser.read(stream));
  }

  TEST_CASE("plus_of_optional_char") {
    auto optional_parser = convert(AnyParser(), [] (auto c) -> optional<char> {
      if(c == '-') {
        return none;
      }
      return c;
    });
    auto parser = +optional_parser;
    auto stream = to_parser_stream("-a-b");
    auto value = std::string();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == "ab");
    REQUIRE(!stream.read());
  }

  TEST_CASE("plus_of_converted_digit_parser") {
    auto int_digit = convert(digit_p, [] (auto c) {
      return c - '0';
    });
    auto parser = +int_digit;
    auto stream = to_parser_stream("12345");
    auto values = std::vector<int>();
    REQUIRE(parser.read(stream, values));
    REQUIRE(values == std::vector{1,2,3,4,5});
    REQUIRE(!stream.read());
  }
}
