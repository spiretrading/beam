#include "Beam/ReactorsTests/MultiReactorTester.hpp"
#include <boost/lexical_cast.hpp>
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/MultiReactor.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace boost;
using namespace std;

void MultiReactorTester::TestNoParameters() {
  auto reactor = MakeMultiReactor(
    [] (const std::vector<const BaseReactor*>& updates) {
      CPPUNIT_ASSERT(updates.empty());
      return 321;
    }, std::vector<BaseReactor*>());
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(321, reactor->Eval());
}

void MultiReactorTester::TestSingleParameter() {
  auto trigger = MakeTriggeredReactor<int>();
  vector<BaseReactor*> reactors = {trigger.get()};
  auto reactor = MakeMultiReactor(
    [&] (const std::vector<const BaseReactor*>& updates) {
      return trigger->Eval() + trigger->Eval();
    }, reactors);
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitializing());
  trigger->SetValue(5);
  trigger->Trigger();
  trigger->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(10, reactor->Eval());
  auto sequence = reactor->GetSequenceNumber();
  trigger->SetComplete();
  trigger->Trigger();
  trigger->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT_EQUAL(sequence, reactor->GetSequenceNumber());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(10, reactor->Eval());
}

void MultiReactorTester::TestTwoParameters() {
  auto xTrigger = MakeTriggeredReactor<string>();
  auto yTrigger = MakeTriggeredReactor<int>();
  vector<BaseReactor*> reactors = {xTrigger.get(), yTrigger.get()};
  auto reactor = MakeMultiReactor(
    [&] (const vector<const BaseReactor*>& updates) {
      return xTrigger->Eval() + lexical_cast<string>(yTrigger->Eval());
    }, reactors);
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitializing());
  xTrigger->SetValue("hello: ");
  xTrigger->Trigger();
  xTrigger->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitializing());
  yTrigger->SetValue(123);
  yTrigger->Trigger();
  yTrigger->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(!reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(string("hello: 123"), reactor->Eval());
  auto sequence = reactor->GetSequenceNumber();
  xTrigger->SetValue("goodbye: ");
  xTrigger->Trigger();
  xTrigger->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(sequence != reactor->GetSequenceNumber());
  CPPUNIT_ASSERT_EQUAL(string("goodbye: 123"), reactor->Eval());
  sequence = reactor->GetSequenceNumber();
  yTrigger->SetValue(321);
  xTrigger->SetComplete();
  yTrigger->SetComplete();
  yTrigger->Trigger();
  yTrigger->Execute();
  xTrigger->Trigger();
  xTrigger->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(sequence != reactor->GetSequenceNumber());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(string("goodbye: 321"), reactor->Eval());
}
