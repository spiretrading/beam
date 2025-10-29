#include <string>
#include <doctest/doctest.h>
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

namespace {
  struct MatchA {
    using Result = void;

    template<IsParserStream S>
    bool read(S& source) const {
      if(!source.read()) {
        return false;
      }
      if(source.peek() == 'a') {
        return true;
      }
      source.undo();
      return false;
    }
  };
}

TEST_SUITE("ConversionParser") {
  TEST_CASE("converter_maps_subparser_value") {
    auto stream = to_parser_stream("5");
    auto parser = convert(AnyParser(), [] (auto c) {
      return c - '0';
    });
    auto value = int();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 5);
    REQUIRE(!stream.read());
  }

  TEST_CASE("converter_void_suppresses_subparser_value") {
    auto stream = to_parser_stream("x");
    auto parser = convert(AnyParser(), [] (auto) {});
    REQUIRE(parser.read(stream));
    REQUIRE(!stream.read());
  }

  TEST_CASE("converter_extends_void_subparser") {
    auto stream = to_parser_stream("a");
    auto parser = convert(MatchA(), [] {
      return 42;
    });
    auto value = int();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == 42);
    REQUIRE(!stream.read());
  }

  TEST_CASE("converter_void_with_void_subparser_suppresses_all") {
    auto stream = to_parser_stream("a");
    auto parser = convert(MatchA(), [] {});
    REQUIRE(parser.read(stream));
    REQUIRE(!stream.read());
  }

  TEST_CASE("cast_helper_performs_static_cast") {
    auto stream = to_parser_stream("A");
    auto parser = cast<int>(AnyParser());
    auto value = int();
    REQUIRE(parser.read(stream, value));
    REQUIRE(value == static_cast<int>('A'));
    REQUIRE(!stream.read());
  }
}
