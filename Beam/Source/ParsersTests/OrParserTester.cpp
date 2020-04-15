#include <boost/variant/get.hpp>
#include <doctest/doctest.h>
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Types.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;

TEST_SUITE("OrParser") {
  TEST_CASE("chaining_no_null_parsers_with_no_duplicate_types") {
    auto parser = ('{' >> *(any_p - '}') >> '}') | int_p | bool_p | alpha_p;
    auto source = ParserStreamFromString("");
    auto value = variant<std::string, int, bool, char>();
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("a");
    REQUIRE(parser.Read(source, value));
    REQUIRE(get<char>(value) == 'a');
  }

  TEST_CASE("chaining_no_null_parsers_with_duplicate_types") {
    auto parser = int_p | bool_p | alpha_p | int_p;
    auto source = ParserStreamFromString("");
    auto value = variant<int, bool, char, int>();
    REQUIRE(!parser.Read(source, value));
    source = ParserStreamFromString("a");
    REQUIRE(parser.Read(source, value));
    REQUIRE(get<char>(value) == 'a');
  }
}
