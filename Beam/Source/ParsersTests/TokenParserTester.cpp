#include "Beam/ParsersTests/TokenParserTester.hpp"
#include "Beam/Parsers/TokenParser.hpp"
#include "Beam/Parsers/Types.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace std;

void TokenParserTester::TestNoSpaceToken() {
  auto parser = Token(+alpha_p);
  auto source = ParserStreamFromString("hello");
  string value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == "hello");
}

void TokenParserTester::TestOneToken() {
  auto parser = Token(+alpha_p);
  auto source = ParserStreamFromString("   hello");
  string value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value == "hello");
}

void TokenParserTester::TestTwoTokens() {
  auto parser = Token(+alpha_p) >> Token(*alpha_p);
  auto source = ParserStreamFromString("   hello   world");
  tuple<string, string> value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(get<0>(value) == "hello");
  CPPUNIT_ASSERT(get<1>(value) == "world");
}

void TokenParserTester::TestStarTokens() {
  auto parser = *Token(+alpha_p);
  auto source = ParserStreamFromString("   hello   world goodbye   sky");
  vector<string> value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value.size() == 4);
  CPPUNIT_ASSERT(value[0] == "hello");
  CPPUNIT_ASSERT(value[1] == "world");
  CPPUNIT_ASSERT(value[2] == "goodbye");
  CPPUNIT_ASSERT(value[3] == "sky");
}

void TokenParserTester::TestChainingTokens() {
  auto parser = tokenize >> +alpha_p >> +alpha_p >> +alpha_p;
  auto source = ParserStreamFromString("   hello   world goodbye");
  tuple<string, string, string> value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(get<0>(value) == "hello");
  CPPUNIT_ASSERT(get<1>(value) == "world");
  CPPUNIT_ASSERT(get<2>(value) == "goodbye");
}
