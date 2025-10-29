#include <doctest/doctest.h>
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

using namespace Beam;

TEST_SUITE("SubParserStream") {
  TEST_CASE("backtrack_on_destruction") {
    auto underlying = to_parser_stream("abc");
    {
      auto sub = SubParserStream(underlying);
      REQUIRE(sub.read());
      REQUIRE(sub.peek() == 'a');
      REQUIRE(sub.read());
      REQUIRE(sub.peek() == 'b');
    }
    REQUIRE(underlying.read());
    REQUIRE(underlying.peek() == 'a');
    REQUIRE(underlying.read());
    REQUIRE(underlying.peek() == 'b');
    REQUIRE(underlying.read());
    REQUIRE(underlying.peek() == 'c');
    REQUIRE(!underlying.read());
  }

  TEST_CASE("accept_prevents_undo") {
    auto underlying = to_parser_stream("abc");
    {
      auto sub = SubParserStream(underlying);
      REQUIRE(sub.read());
      REQUIRE(sub.peek() == 'a');
      REQUIRE(sub.read());
      REQUIRE(sub.peek() == 'b');
      sub.accept();
    }
    REQUIRE(underlying.read());
    REQUIRE(underlying.peek() == 'c');
    REQUIRE(!underlying.read());
  }

  TEST_CASE("undo_inside_sub_and_backtrack_on_destruction") {
    auto underlying = to_parser_stream("ab");
    {
      auto sub = SubParserStream(underlying);
      REQUIRE(sub.read());
      REQUIRE(sub.peek() == 'a');
      REQUIRE(sub.read());
      REQUIRE(sub.peek() == 'b');
      sub.undo();
      REQUIRE(sub.peek() == 'a');
    }
    REQUIRE(underlying.read());
    REQUIRE(underlying.peek() == 'a');
    REQUIRE(underlying.read());
    REQUIRE(underlying.peek() == 'b');
    REQUIRE(!underlying.read());
  }

  TEST_CASE("empty_stream_no_crash") {
    auto underlying = to_parser_stream("");
    {
      auto sub = SubParserStream(underlying);
      REQUIRE(!sub.read());
    }
    REQUIRE(!underlying.read());
  }
}
