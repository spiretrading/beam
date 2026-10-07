#include <limits>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

namespace {
  struct DecimalValue {
    double m_value;

    DecimalValue() noexcept
      : m_value(0) {}

    explicit DecimalValue(double value) noexcept
      : m_value(value) {}
  };
}

TEST_SUITE("DecimalParser") {
  TEST_CASE("positive_decimal") {
    auto parser = DecimalParser<double>();
    auto source = to_parser_stream("3.300000");
    auto value = double();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == 3.3);
    source = to_parser_stream("1.a32");
    REQUIRE(!parser.read(source, value));
  }

  TEST_CASE("negative_decimal") {
    auto parser = DecimalParser<double>();
    auto source = to_parser_stream("-3.1415");
    auto value = double();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == -3.1415);
    source = to_parser_stream("-a3.123");
    REQUIRE(!parser.read(source, value));
  }

  TEST_CASE("exponents_and_prefixes") {
    for(auto& [text, expected] :
        std::vector<std::pair<std::string, double>>({
          {"1e25", 1e25}, {"-1E+25", -1e25}, {"1.25e-3", 0.00125},
          {"0e9999", 0}, {"2E0", 2}, {"3", 3}, {"-3.5", -3.5}})) {
      CAPTURE(text);
      for(auto& suffix : {"", ","}) {
        auto source = to_parser_stream(text + suffix);
        auto value = double();
        REQUIRE(double_p.read(source, value));
        REQUIRE(value == expected);
        if(*suffix != '\0') {
          REQUIRE(source.read());
          REQUIRE(source.peek() == ',');
        } else {
          REQUIRE(!source.read());
        }
        source = to_parser_stream(text + suffix);
        REQUIRE(double_p.read(source));
        if(*suffix != '\0') {
          REQUIRE(source.read());
          REQUIRE(source.peek() == ',');
        } else {
          REQUIRE(!source.read());
        }
      }
    }
  }

  TEST_CASE("invalid_numbers") {
    for(auto& text : {"", "-", "1.", "1e", "1e+", "1e-", "1.e2",
        "1e+x", "1E--2", "1e9999", "1e-9999"}) {
      CAPTURE(text);
      auto source = to_parser_stream(text);
      auto value = 7.0;
      REQUIRE(!double_p.read(source, value));
      REQUIRE(value == 7);
      if(*text != '\0') {
        REQUIRE(source.read());
        REQUIRE(source.peek() == *text);
      }
      source = to_parser_stream(text);
      REQUIRE(!double_p.read(source));
      if(*text != '\0') {
        REQUIRE(source.read());
        REQUIRE(source.peek() == *text);
      }
    }
    auto source = to_parser_stream("1e39");
    auto value = float();
    REQUIRE(!float_p.read(source, value));
    source = to_parser_stream("1e39");
    REQUIRE(!float_p.read(source));
  }

  TEST_CASE("representable_limits") {
    auto source = to_parser_stream("1.7976931348623157e308");
    auto value = double();
    REQUIRE(double_p.read(source, value));
    REQUIRE(value == std::numeric_limits<double>::max());
    source = to_parser_stream("4.9406564584124654e-324");
    REQUIRE(double_p.read(source, value));
    REQUIRE(value == std::numeric_limits<double>::denorm_min());
  }

  TEST_CASE("custom_numeric_type") {
    auto parser = DecimalParser<DecimalValue>();
    auto source = to_parser_stream("-1.25e2,");
    auto value = DecimalValue(7);
    REQUIRE(parser.read(source, value));
    REQUIRE(value.m_value == -125);
    REQUIRE(source.read());
    REQUIRE(source.peek() == ',');
    source = to_parser_stream("1e9999");
    REQUIRE(!parser.read(source, value));
    REQUIRE(value.m_value == -125);
    REQUIRE(source.read());
    REQUIRE(source.peek() == '1');
    source = to_parser_stream("3.5");
    REQUIRE(parser.read(source));
  }

}
