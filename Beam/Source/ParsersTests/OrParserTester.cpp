#include <boost/variant/get.hpp>
#include <doctest/doctest.h>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/BoolParser.hpp"
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/DifferenceParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/OrParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/StarParser.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("OrParser") {
  TEST_CASE("chaining_no_null_parsers_with_no_duplicate_types") {
    auto parser = ('{' >> *(any_p - '}') >> '}') | int_p | bool_p | alpha_p;
    auto source = to_parser_stream("");
    auto value = variant<std::string, int, bool, char>();
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a");
    REQUIRE(parser.read(source, value));
    REQUIRE(get<char>(value) == 'a');
  }

  TEST_CASE("chaining_no_null_parsers_with_duplicate_types") {
    auto parser = int_p | bool_p | alpha_p | int_p;
    auto source = to_parser_stream("");
    auto value = variant<int, bool, char, int>();
    REQUIRE(!parser.read(source, value));
    source = to_parser_stream("a");
    REQUIRE(parser.read(source, value));
    REQUIRE(get<char>(value) == 'a');
  }
}
