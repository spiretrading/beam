#include "Beam/ParsersTests/DecimalParserTester.hpp"
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace std;

void DecimalParserTester::TestPositiveDecimal() {
  DecimalParser<double> parser;
  auto source = ParserStreamFromString("3.300000");
  double value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == 3.3);
  source = ParserStreamFromString("1.a32");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}

void DecimalParserTester::TestNegativeDecimal() {
  DecimalParser<double> parser;
  auto source = ParserStreamFromString("-3.1415");
  double value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == -3.1415);
  source = ParserStreamFromString("-a3.123");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}
