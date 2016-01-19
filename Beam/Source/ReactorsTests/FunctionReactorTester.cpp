#include "Beam/ReactorsTests/FunctionReactorTester.hpp"
#include <boost/lexical_cast.hpp>
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace boost;
using namespace std;

void FunctionReactorTester::TestNoParameters() {
  auto reactor = MakeFunctionReactor(
    [] {
      return 321;
    });
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(321, reactor->Eval());
}

void FunctionReactorTester::TestSingleParameter() {
  auto trigger = MakeTriggeredReactor<int>();
  auto reactor = MakeFunctionReactor(
    [] (int n) {
      return n + n;
    }, trigger);
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

void FunctionReactorTester::TestTwoParameters() {
  auto xTrigger = MakeTriggeredReactor<string>();
  auto yTrigger = MakeTriggeredReactor<int>();
  auto reactor = MakeFunctionReactor(
    [] (const string& x, int y) {
      return x + lexical_cast<string>(y);
    }, xTrigger, yTrigger);
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

void FunctionReactorTester::TestRange() {
  auto trigger = MakeTriggeredReactor<int>();
  trigger->SetValue(0);
  trigger->Trigger();
  trigger->Execute();
  bool isInitialized = false;
  auto rangeReactor = MakeFunctionReactor(
    [=] (int lower, int upper, int current) mutable {
      if(isInitialized) {
        if(current >= upper) {
          return current;
        }
        if(current + 1 == upper) {
          trigger->SetComplete();
        } else {
          trigger->SetValue(current + 1);
        }
        trigger->Trigger();
        trigger->Execute();
        return current + 1;
      }
      isInitialized = true;
      trigger->SetValue(lower);
      trigger->Trigger();
      trigger->Execute();
      return lower;
    }, MakeConstantReactor(0), MakeConstantReactor(5), trigger);
  rangeReactor->Commit();
  CPPUNIT_ASSERT_EQUAL(0, rangeReactor->Eval());
  rangeReactor->Commit();
  CPPUNIT_ASSERT_EQUAL(1, rangeReactor->Eval());
  rangeReactor->Commit();
  CPPUNIT_ASSERT_EQUAL(2, rangeReactor->Eval());
  rangeReactor->Commit();
  CPPUNIT_ASSERT_EQUAL(3, rangeReactor->Eval());
  rangeReactor->Commit();
  CPPUNIT_ASSERT_EQUAL(4, rangeReactor->Eval());
  rangeReactor->Commit();
  CPPUNIT_ASSERT_EQUAL(5, rangeReactor->Eval());
  auto sequence = rangeReactor->GetSequenceNumber();
  rangeReactor->Commit();
  CPPUNIT_ASSERT_EQUAL(sequence, rangeReactor->GetSequenceNumber());
  CPPUNIT_ASSERT(rangeReactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(5, rangeReactor->Eval());
}
