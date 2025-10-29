#include <doctest/doctest.h>
#include "Beam/Collections/EnumIterator.hpp"
#include "Beam/Parsers/EnumeratorParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;

namespace {
  BEAM_ENUM(Fruit,
    APPLE,
    PEAR,
    BANANA);
}

TEST_SUITE("EnumeratorParser") {
  TEST_CASE("read_enumerator") {
    auto parser =
      EnumeratorParser(begin(make_range<Fruit>()), end(make_range<Fruit>()));
    auto source = to_parser_stream("APPLE");
    auto value = Fruit();
    REQUIRE(parser.read(source, value));
    REQUIRE(value == Fruit::APPLE);
    source = to_parser_stream("CAT");
    REQUIRE(!parser.read(source, value));
  }
}
