#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/TasksTests/ChainedTaskTester.hpp"
#include "Beam/TasksTests/FunctionTaskTester.hpp"
#include "Beam/TasksTests/IdleTaskTester.hpp"
#include "Beam/TasksTests/PackagedTaskTester.hpp"
#include "Beam/TasksTests/ReactorTaskTester.hpp"
#include "Beam/TasksTests/SpawnTaskTester.hpp"
#include "Beam/TasksTests/UntilTaskTester.hpp"
#include "Beam/TasksTests/WhenTaskTester.hpp"

using namespace Beam::Tasks::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(IdleTaskTester::suite());
  runner.addTest(PackagedTaskTester::suite());
  runner.addTest(FunctionTaskTester::suite());
  runner.addTest(ChainedTaskTester::suite());
  runner.addTest(UntilTaskTester::suite());
  runner.addTest(WhenTaskTester::suite());
  runner.addTest(SpawnTaskTester::suite());
  runner.addTest(ReactorTaskTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  auto wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
