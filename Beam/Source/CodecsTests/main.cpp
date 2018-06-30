#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/CodecsTests/CodedReaderTester.hpp"
#include "Beam/CodecsTests/CodedWriterTester.hpp"
#include "Beam/CodecsTests/NullDecoderTester.hpp"
#include "Beam/CodecsTests/NullEncoderTester.hpp"
#include "Beam/CodecsTests/SizeDeclarativeDecoderTester.hpp"
#include "Beam/CodecsTests/SizeDeclarativeEncoderTester.hpp"
#include "Beam/CodecsTests/ZLibCodecTester.hpp"

using namespace Beam::Codecs::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(CodedReaderTester::suite());
  runner.addTest(CodedWriterTester::suite());
  runner.addTest(NullDecoderTester::suite());
  runner.addTest(NullEncoderTester::suite());
  runner.addTest(SizeDeclarativeDecoderTester::suite());
  runner.addTest(SizeDeclarativeEncoderTester::suite());
  runner.addTest(ZLibCodecTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  auto wasSucessful = runner.run();
  return wasSucessful ? 0 : 1;
}
