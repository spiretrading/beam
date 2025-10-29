#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/NotParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("NotParser") {
  TEST_CASE("fail_not_int") {
    auto parser = !int_p;
    auto source = to_parser_stream("123");
    auto c = optional<char>();
    REQUIRE(!parser.read(source, c));
    REQUIRE(!c);
  }

  TEST_CASE("match_not_int") {
    auto parser = !int_p;
    auto source = to_parser_stream("a123");
    auto c = optional<char>();
    REQUIRE(parser.read(source, c));
    REQUIRE(c == 'a');
  }
}
