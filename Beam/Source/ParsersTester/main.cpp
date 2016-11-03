#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/ParsersTests/DateParserTester.hpp"
#include "Beam/ParsersTests/DateTimeParserTester.hpp"
#include "Beam/ParsersTests/DecimalParserTester.hpp"
#include "Beam/ParsersTests/ConcatenateParserTester.hpp"
#include "Beam/ParsersTests/EnumeratorParserTester.hpp"
#include "Beam/ParsersTests/IntegralParserTester.hpp"
#include "Beam/ParsersTests/ListParserTester.hpp"
#include "Beam/ParsersTests/OrParserTester.hpp"
#include "Beam/ParsersTests/RationalParserTester.hpp"
#include "Beam/ParsersTests/TimeDurationParserTester.hpp"
#include "Beam/ParsersTests/TokenParserTester.hpp"

using namespace Beam::Parsers::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.eventManager().addListener(&listener);
  runner.addTest(IntegralParserTester::suite());
  runner.addTest(DecimalParserTester::suite());
  runner.addTest(ConcatenateParserTester::suite());
  runner.addTest(OrParserTester::suite());
  runner.addTest(ListParserTester::suite());
  runner.addTest(TokenParserTester::suite());
  runner.addTest(EnumeratorParserTester::suite());
  runner.addTest(TimeDurationParserTester::suite());
  runner.addTest(DateParserTester::suite());
  runner.addTest(DateTimeParserTester::suite());
  runner.addTest(RationalParserTester::suite());
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : 1;
}
