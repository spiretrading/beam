#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/ReactorsTests/AggregateReactorTester.hpp"
#include "Beam/ReactorsTests/AlarmReactorTester.hpp"
#include "Beam/ReactorsTests/ChainReactorTester.hpp"
#include "Beam/ReactorsTests/ConstantReactorTester.hpp"
#include "Beam/ReactorsTests/FilterReactorTester.hpp"
#include "Beam/ReactorsTests/FoldReactorTester.hpp"
#include "Beam/ReactorsTests/FunctionReactorTester.hpp"
#include "Beam/ReactorsTests/MultiReactorTester.hpp"
#include "Beam/ReactorsTests/NoneReactorTester.hpp"
#include "Beam/ReactorsTests/PublisherReactorTester.hpp"
#include "Beam/ReactorsTests/SwitchReactorTester.hpp"
#include "Beam/ReactorsTests/TriggeredReactorTester.hpp"
#include "Beam/ReactorsTests/TriggerTester.hpp"
#include "Beam/ReactorsTests/WeakReactorTester.hpp"

using namespace Beam::Reactors::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(NoneReactorTester::suite());
  runner.addTest(ConstantReactorTester::suite());
  runner.addTest(TriggeredReactorTester::suite());
  runner.addTest(FunctionReactorTester::suite());
  runner.addTest(MultiReactorTester::suite());
  runner.addTest(WeakReactorTester::suite());
  runner.addTest(PublisherReactorTester::suite());
  runner.addTest(SwitchReactorTester::suite());
  runner.addTest(AlarmReactorTester::suite());
  runner.addTest(TriggerTester::suite());
  runner.addTest(AggregateReactorTester::suite());
  runner.addTest(FilterReactorTester::suite());
  runner.addTest(FoldReactorTester::suite());
  runner.addTest(ChainReactorTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
