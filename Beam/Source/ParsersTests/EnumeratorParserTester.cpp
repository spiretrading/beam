#include <doctest/doctest.h>
#include "Beam/Collections/EnumIterator.hpp"
#include "Beam/Parsers/EnumeratorParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::Parsers;

namespace {
  BEAM_ENUM(Fruit,
    APPLE,
    PEAR,
    BANANA);

  std::ostream& operator <<(std::ostream& out, Fruit fruit) {
    if(fruit == Fruit::APPLE) {
      return out << "APPLE";
    } else if(fruit == Fruit::PEAR) {
      return out << "PEAR";
    } else if(fruit == Fruit::BANANA) {
      return out << "BANANA";
    }
    return out << "NONE";
  }
}

TEST_SUITE("EnumeratorParser") {
  TEST_CASE("read_enumerator") {
    auto parser = EnumeratorParser<Fruit>(begin(MakeRange<Fruit>()),
      end(MakeRange<Fruit>()));
    auto source = ParserStreamFromString("APPLE");
    auto value = Fruit();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == Fruit::APPLE);
    source = ParserStreamFromString("CAT");
    REQUIRE(!parser.Read(source, value));
  }
}
