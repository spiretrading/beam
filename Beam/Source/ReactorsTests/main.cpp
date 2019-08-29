#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/ReactorsTests/TimerReactorTester.hpp"

using namespace Beam::Reactors::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(QueueReactorTester::suite());
  runner.addTest(TimerReactorTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  auto wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
