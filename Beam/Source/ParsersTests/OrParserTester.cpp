#include "Beam/ParsersTests/OrParserTester.hpp"
#include <boost/variant/get.hpp>
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Types.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Parsers::Tests;
using namespace boost;

void OrParserTester::TestChainingNoNullParsersWithNoDuplicateTypes() {
  auto parser = ('{' >> *(any_p - '}') >> '}') | int_p | bool_p | alpha_p;
  auto source = ParserStreamFromString("");
  variant<std::string, int, bool, char> value;
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("a");
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(get<char>(value) == 'a');
}

void OrParserTester::TestChainingNoNullParsersWithDuplicateTypes() {
  auto parser = int_p | bool_p | alpha_p | int_p;
  auto source = ParserStreamFromString("");
  variant<int, bool, char, int> value;
  CPPUNIT_ASSERT(!parser.Read(source, value));
  source = ParserStreamFromString("a");
  CPPUNIT_ASSERT(parser.Read(source, value));
  CPPUNIT_ASSERT(get<char>(value) == 'a');
}
