#include "Beam/ParsersTests/TimeDurationParserTester.hpp"
#include "Beam/Parsers/TimeDurationParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

void TimeDurationParserTester::TestReadTimeDuration() {
  TimeDurationParser parser;
  auto source = ParserStreamFromString("1:2:3.4");
  time_duration value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == hours(1) + minutes(2) + seconds(3) +
    milliseconds(400));
  source = ParserStreamFromString("a:1:b");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}
