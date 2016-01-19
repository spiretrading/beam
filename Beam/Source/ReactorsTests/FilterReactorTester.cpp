#include "Beam/ReactorsTests/FilterReactorTester.hpp"
#include "Beam/Reactors/FilterReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;

void FilterReactorTester::TestIndependentUpdates() {
  auto filter = MakeTriggeredReactor<bool>();
  auto source = MakeTriggeredReactor<int>();
  auto reactor = MakeFilterReactor(filter, source);
  filter->SetValue(true);
  filter->Trigger();
  filter->Execute();
  source->SetValue(123);
  source->Trigger();
  source->Execute();
  reactor->Commit();
  auto sequenceA = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(!reactor->IsInitializing());
  CPPUNIT_ASSERT(reactor->Eval() == 123);
  source->SetValue(321);
  source->Trigger();
  source->Execute();
  reactor->Commit();
  auto sequenceB = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceB > sequenceA);
  CPPUNIT_ASSERT(reactor->Eval() == 321);
  filter->SetValue(false);
  filter->Trigger();
  filter->Execute();
  reactor->Commit();
  auto sequenceC = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceC == sequenceB);
  CPPUNIT_ASSERT(reactor->Eval() == 321);
  source->SetValue(1024);
  source->Trigger();
  source->Execute();
  reactor->Commit();
  auto sequenceD = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceD == sequenceC);
  CPPUNIT_ASSERT(reactor->Eval() == 321);
  filter->SetValue(true);
  filter->Trigger();
  filter->Execute();
  reactor->Commit();
  auto sequenceE = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceE > sequenceD);
  CPPUNIT_ASSERT(reactor->Eval() == 1024);
}

void FilterReactorTester::TestDependentUpdates() {
  auto filter = MakeTriggeredReactor<bool>();
  auto source = MakeTriggeredReactor<int>();
  auto reactor = MakeFilterReactor(filter, source);
  filter->SetValue(true);
  filter->Trigger();
  filter->Execute();
  source->SetValue(123);
  source->Trigger();
  source->Execute();
  reactor->Commit();
  auto sequenceA = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(!reactor->IsInitializing());
  CPPUNIT_ASSERT(reactor->Eval() == 123);
  source->SetValue(321);
  source->Trigger();
  source->Execute();
  filter->SetValue(false);
  filter->Trigger();
  filter->Execute();
  reactor->Commit();
  auto sequenceB = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceB = sequenceA);
  CPPUNIT_ASSERT(reactor->Eval() == 123);
  source->SetValue(1024);
  source->Trigger();
  source->Execute();
  filter->SetValue(true);
  filter->Trigger();
  filter->Execute();
  reactor->Commit();
  auto sequenceC = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceC > sequenceB);
  CPPUNIT_ASSERT(reactor->Eval() == 1024);
}
