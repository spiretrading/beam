#include <doctest/doctest.h>
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

TEST_SUITE("ReaderParserStream") {
  TEST_CASE("read_entire_string") {
    auto stream = to_parser_stream("abc");
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'a');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'b');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'c');
    REQUIRE(!stream.read());
  }

  TEST_CASE("undo_single_character") {
    auto stream = to_parser_stream("ab");
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'a');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'b');
    stream.undo();
    REQUIRE(stream.peek() == 'a');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'b');
    REQUIRE(!stream.read());
  }

  TEST_CASE("undo_multiple_characters") {
    auto stream = to_parser_stream("abcd");
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'a');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'b');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'c');
    stream.undo(2);
    REQUIRE(stream.peek() == 'a');
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'b');
  }

  TEST_CASE("read_empty_string") {
    auto stream = to_parser_stream("");
    REQUIRE(!stream.read());
  }

  TEST_CASE("read_long_string") {
    auto length = std::size_t(2000);
    auto source = std::string(length, 'A');
    auto stream = to_parser_stream(source);
    auto count = std::size_t(0);
    while(true) {
      if(!stream.read()) {
        break;
      }
      ++count;
      REQUIRE(stream.peek() == 'A');
    }
    REQUIRE(count == length);
  }

  TEST_CASE("accept_after_read") {
    auto stream = to_parser_stream("a");
    REQUIRE(stream.read());
    REQUIRE(stream.peek() == 'a');
    stream.accept();
    REQUIRE(!stream.read());
  }
}
