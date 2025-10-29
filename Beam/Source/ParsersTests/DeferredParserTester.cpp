#include <doctest/doctest.h>
#include "Beam/Parsers/DeferredParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("DeferredParser") {
  TEST_CASE("default_construct") {
    auto parser = DeferredParser<int>();
    auto stream = to_parser_stream(from<SharedBuffer>("123"));
    auto value = int();
    REQUIRE(!parser.read(stream, value));
    REQUIRE(!parser.read(stream));
    parser.set(int_p);
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 123);
    REQUIRE(!stream.read());
  }

  TEST_CASE("construct_with_parser_parses_value") {
    auto parser = DeferredParser(int_p);
    auto stream = to_parser_stream(from<SharedBuffer>("-42"));
    auto value = int();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == -42);
    REQUIRE(!stream.read());
  }

  TEST_CASE("multiple_set") {
    auto parser = DeferredParser<int>();
    parser.set(int_p);
    {
      auto stream = to_parser_stream("7");
      auto value = int();
      REQUIRE(parser.read(stream, value));
      REQUIRE(value == 7);
    }
    parser.set(digit_p);
    {
      auto stream = to_parser_stream("8x");
      auto value = int();
      REQUIRE(parser.read(stream, value));
      REQUIRE(value == '8');
      REQUIRE(stream.read());
      REQUIRE(stream.peek() == 'x');
    }
  }

  TEST_CASE("read_without_value") {
    auto parser = DeferredParser(int_p);
    auto stream = to_parser_stream("56z");
    REQUIRE(parser.read(stream));
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'z');
  }
}
