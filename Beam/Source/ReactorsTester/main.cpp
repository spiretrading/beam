#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/ReactorsTests/BasicReactorTester.hpp"
#include "Beam/ReactorsTests/ChainReactorTester.hpp"
#include "Beam/ReactorsTests/ConstantReactorTester.hpp"
#include "Beam/ReactorsTests/DoReactorTester.hpp"
#include "Beam/ReactorsTests/FirstReactorTester.hpp"
#include "Beam/ReactorsTests/FunctionReactorTester.hpp"
#include "Beam/ReactorsTests/LastReactorTester.hpp"
#include "Beam/ReactorsTests/MultiReactorTester.hpp"
#include "Beam/ReactorsTests/NoneReactorTester.hpp"
#include "Beam/ReactorsTests/NonRepeatingReactorTester.hpp"
#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/ReactorsTests/ReactorMonitorTester.hpp"
#include "Beam/ReactorsTests/TimerReactorTester.hpp"
#include "Beam/ReactorsTests/TriggerTester.hpp"
#include "Beam/ReactorsTests/UpdateReactorTester.hpp"

using namespace Beam::Reactors::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(NoneReactorTester::suite());
  runner.addTest(ConstantReactorTester::suite());
  runner.addTest(TriggerTester::suite());
  runner.addTest(BasicReactorTester::suite());
  runner.addTest(ChainReactorTester::suite());
  runner.addTest(UpdateReactorTester::suite());
  runner.addTest(QueueReactorTester::suite());
  runner.addTest(FunctionReactorTester::suite());
  runner.addTest(MultiReactorTester::suite());
  runner.addTest(NonRepeatingReactorTester::suite());
  runner.addTest(DoReactorTester::suite());
  runner.addTest(FirstReactorTester::suite());
  runner.addTest(LastReactorTester::suite());
  runner.addTest(TimerReactorTester::suite());
  runner.addTest(ReactorMonitorTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
