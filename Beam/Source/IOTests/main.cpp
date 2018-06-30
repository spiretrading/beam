#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/IOTests/BufferReaderTester.hpp"
#include "Beam/IOTests/LocalServerConnectionTester.hpp"
#include "Beam/IOTests/PipedReaderWriterTester.hpp"
#include "Beam/IOTests/SharedBufferTester.hpp"
#include "Beam/IOTests/SizeDeclarativeReaderTester.hpp"

using namespace Beam::IO::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(SharedBufferTester::suite());
  runner.addTest(BufferReaderTester::suite());
  runner.addTest(PipedReaderWriterTester::suite());
  runner.addTest(LocalServerConnectionTester::suite());
  runner.addTest(SizeDeclarativeReaderTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  auto wasSucessful = runner.run();
  return wasSucessful ? 0 : 1;
}
