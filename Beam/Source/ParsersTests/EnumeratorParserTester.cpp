#include "Beam/ParsersTests/EnumeratorParserTester.hpp"
#include "Beam/Collections/EnumIterator.hpp"
#include "Beam/Parsers/EnumeratorParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace std;

namespace {
  BEAM_ENUM(Fruit,
    APPLE,
    PEAR,
    BANANA);

  string ToString(Fruit fruit) {
    if(fruit == Fruit::APPLE) {
      return "APPLE";
    } else if(fruit == Fruit::PEAR) {
      return "PEAR";
    } else if(fruit == Fruit::BANANA) {
      return "BANANA";
    }
    return "NONE";
  }
}

namespace Beam {
  template<>
  struct EnumeratorCount<Fruit> : std::integral_constant<int, 3> {};
}

void EnumeratorParserTester::TestReadEnumerator() {
  EnumeratorParser<Fruit> parser(begin(MakeRange<Fruit>()),
    end(MakeRange<Fruit>()), static_cast<string (*)(Fruit)>(ToString));
  auto source = ParserStreamFromString("APPLE");
  Fruit value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == Fruit::APPLE);
  source = ParserStreamFromString("CAT");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}
