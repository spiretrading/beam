#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Beam/Parsers/NotParser.hpp"
#include "Beam/Parsers/Types.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;

TEST_SUITE("NotParser") {
  TEST_CASE("fail_not_int") {
    auto parser = Not(int_p);
    auto source = ParserStreamFromString("123");
    auto c = optional<char>();
    REQUIRE(!parser.Read(source, c));
    REQUIRE_FALSE(c.is_initialized());
  }

  TEST_CASE("match_not_int") {
    auto parser = Not(int_p);
    auto source = ParserStreamFromString("a123");
    auto c = optional<char>();
    REQUIRE(parser.Read(source, c));
    REQUIRE(c == 'a');
  }
}
