#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/ReactorsTests/ConstantReactorTester.hpp"
#include "Beam/ReactorsTests/DoReactorTester.hpp"
#include "Beam/ReactorsTests/FunctionReactorTester.hpp"
#include "Beam/ReactorsTests/NoneReactorTester.hpp"
#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/ReactorsTests/TimerReactorTester.hpp"
#include "Beam/ReactorsTests/TriggerTester.hpp"

using namespace Beam::Reactors::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(NoneReactorTester::suite());
  runner.addTest(ConstantReactorTester::suite());
  runner.addTest(TriggerTester::suite());
  runner.addTest(QueueReactorTester::suite());
  runner.addTest(FunctionReactorTester::suite());
  runner.addTest(DoReactorTester::suite());
  runner.addTest(TimerReactorTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
