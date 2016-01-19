#include "Beam/ParsersTests/ConcatenateParserTester.hpp"
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Types.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace std;

void ConcatenateParserTester::TestVoidParsers() {
  auto parser = eps_p >> 'a' >> 'b' >> 'c';
  auto source = ParserStreamFromString("");
  bool result = parser.Read(source);
  CPPUNIT_ASSERT(!result);
  source = ParserStreamFromString("a");
  result = parser.Read(source);
  CPPUNIT_ASSERT(!result);
  source = ParserStreamFromString("ab");
  result = parser.Read(source);
  CPPUNIT_ASSERT(!result);
  source = ParserStreamFromString("abc");
  result = parser.Read(source);
  CPPUNIT_ASSERT(result);
  source = ParserStreamFromString("abcd");
  result = parser.Read(source);
  CPPUNIT_ASSERT(result);
  source = ParserStreamFromString("dabc");
  result = parser.Read(source);
  CPPUNIT_ASSERT(!result);
}

void ConcatenateParserTester::TestLeftVoidParsers() {
  auto parser = 'a' >> int_p;
  auto source = ParserStreamFromString("");
  int value;
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("a");
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("a5");
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == 5);
}

void ConcatenateParserTester::TestRightVoidParsers() {
  auto parser = int_p >> 'a';
  auto source = ParserStreamFromString("");
  int value;
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("a");
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("2a");
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == 2);
}

void ConcatenateParserTester::TestNoVoidParsers() {
  auto parser = int_p >> any_p;
  auto source = ParserStreamFromString("");
  std::tuple<int, char> value;
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("a");
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("24a");
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(get<0>(value) == 24 && get<1>(value) == 'a');
}
