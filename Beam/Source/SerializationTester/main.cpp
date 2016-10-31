#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/SerializationTests/BinaryShuttleTester.hpp"
#include "Beam/SerializationTests/JsonShuttleTester.hpp"
#include "Beam/SerializationTests/ShuttleVariantTester.hpp"

using namespace Beam::Serialization::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.eventManager().addListener(&listener);
  runner.addTest(BinaryShuttleTester::suite());
  runner.addTest(JsonShuttleTester::suite());
  runner.addTest(ShuttleVariantTester::suite());
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  bool wasSucessful = runner.run();
  return wasSucessful ? 0 : 1;
}
