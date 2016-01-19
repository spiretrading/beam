#include "Beam/ParsersTests/DateParserTester.hpp"
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void DateParserTester::TestReadDate() {
  DateParser parser;
  auto source = ParserStreamFromString("2003-02-11");
  date value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == date(2003, 2, 11));
  source = ParserStreamFromString("2003-02-ab");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}
