#include "Beam/ParsersTests/ListParserTester.hpp"
#include "Beam/Parsers/Types.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace std;

void ListParserTester::TestSingleIntList() {
  auto parser = List(int_p, ',') >> Discard(" .");
  auto source = ParserStreamFromString("123 .");
  vector<int> values;
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 1);
  CPPUNIT_ASSERT(values.front() == 123);
}

void ListParserTester::TestTwoIntList() {
  auto parser = List(int_p, ',') >> Discard(" .");
  auto source = ParserStreamFromString("123,456 .");
  vector<int> values;
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 2);
  CPPUNIT_ASSERT(values[0] == 123);
  CPPUNIT_ASSERT(values[1] == 456);
  source = ParserStreamFromString("321   ,654 .");
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 2);
  CPPUNIT_ASSERT(values[0] == 321);
  CPPUNIT_ASSERT(values[1] == 654);
  source = ParserStreamFromString("98,   89 .");
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 2);
  CPPUNIT_ASSERT(values[0] == 98);
  CPPUNIT_ASSERT(values[1] == 89);
  source = ParserStreamFromString("14  ,   52 .");
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 2);
  CPPUNIT_ASSERT(values[0] == 14);
  CPPUNIT_ASSERT(values[1] == 52);
}

void ListParserTester::TestThreeIntList() {
  auto parser = List(int_p, ',') >> Discard(" .");
  auto source = ParserStreamFromString("123,456,789 .");
  vector<int> values;
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 3);
  CPPUNIT_ASSERT(values[0] == 123);
  CPPUNIT_ASSERT(values[1] == 456);
  CPPUNIT_ASSERT(values[2] == 789);
  source = ParserStreamFromString("321,654  ,987 .");
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 3);
  CPPUNIT_ASSERT(values[0] == 321);
  CPPUNIT_ASSERT(values[1] == 654);
  CPPUNIT_ASSERT(values[2] == 987);
  source = ParserStreamFromString("98,89,   12 .");
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 3);
  CPPUNIT_ASSERT(values[0] == 98);
  CPPUNIT_ASSERT(values[1] == 89);
  CPPUNIT_ASSERT(values[2] == 12);
  source = ParserStreamFromString("14,52  ,   42 .");
  CPPUNIT_ASSERT(parser.Read(source, values));
  CPPUNIT_ASSERT(values.size() == 3);
  CPPUNIT_ASSERT(values[0] == 14);
  CPPUNIT_ASSERT(values[1] == 52);
  CPPUNIT_ASSERT(values[2] == 42);
}
