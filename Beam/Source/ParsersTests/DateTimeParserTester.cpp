#include "Beam/ParsersTests/DateTimeParserTester.hpp"
#include "Beam/Parsers/DateTimeParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace std;

void DateTimeParserTester::TestReadDateTime() {
  DateTimeParser parser;
  auto source = ParserStreamFromString("2003-02-11 13:04:53.1");
  ptime value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value.date() == date(2003, 2, 11));
  CPPUNIT_ASSERT(value.time_of_day() == hours(13) + minutes(4) + seconds(53) +
    milliseconds(100));
  source = ParserStreamFromString("2003-02-11");
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("13:04:53.1");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}
