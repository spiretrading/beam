#include "Beam/ParsersTests/RationalParserTester.hpp"
#include "Beam/Parsers/RationalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;
using namespace std;

void RationalParserTester::TestWholeNumbers() {
  RationalParser<int> parser;
  auto source = ParserStreamFromString("5");
  rational<int> value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value.numerator() == 5);
  CPPUNIT_ASSERT(value.denominator() == 1);
  source = ParserStreamFromString("-2");
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value.numerator() == -2);
  CPPUNIT_ASSERT(value.denominator() == 1);
  source = ParserStreamFromString("-a5");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}

void RationalParserTester::TestWithinZeroAndOne() {
  RationalParser<int> parser;
  auto source = ParserStreamFromString("0.5");
  rational<int> value;
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value.numerator() == 1);
  CPPUNIT_ASSERT(value.denominator() == 2);
  source = ParserStreamFromString("-0.5");
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(value.numerator() == -1);
  CPPUNIT_ASSERT(value.denominator() == 2);
  source = ParserStreamFromString("-.a5");
  CPPUNIT_ASSERT(!parser.Read(source, value));
}
