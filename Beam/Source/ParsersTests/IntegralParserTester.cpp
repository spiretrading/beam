#include "Beam/ParsersTests/IntegralParserTester.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace std;

void IntegralParserTester::TestPositiveInt() {
  IntegralParser<int> parser;
  auto source = ParserStreamFromString("123");
  int value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == 123);
  source = ParserStreamFromString("a123");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}

void IntegralParserTester::TestNegativeInt() {
  IntegralParser<int> parser;
  auto source = ParserStreamFromString("-123");
  int value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == -123);
  source = ParserStreamFromString("-a123");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}
