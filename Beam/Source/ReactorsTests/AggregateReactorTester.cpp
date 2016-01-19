#include "Beam/ReactorsTests/AggregateReactorTester.hpp"
#include "Beam/Reactors/AggregateReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/NoneReactor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;

void AggregateReactorTester::TestNoneProducer() {
  auto reactor = MakeAggregateReactor(MakeNoneReactor<ConstantReactor<int>*>());
  auto sequence = reactor->GetSequenceNumber();
  reactor->Commit();
  CPPUNIT_ASSERT_EQUAL(sequence, reactor->GetSequenceNumber());
  CPPUNIT_ASSERT(reactor->IsComplete());
}

void AggregateReactorTester::TestSimultaneousUpdate() {
  auto producer = MakeTriggeredReactor<std::shared_ptr<Reactor<int>>>();
  auto reactor = MakeAggregateReactor(producer);
  auto triggerA = MakeTriggeredReactor<int>();
  producer->SetValue(triggerA);
  producer->Trigger();
  producer->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitializing());
  triggerA->SetValue(123);
  triggerA->Trigger();
  triggerA->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(!reactor->IsInitializing());
  auto sequenceA = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(reactor->Eval() == 123);
  auto triggerB = MakeTriggeredReactor<int>();
  producer->SetValue(triggerB);
  producer->Trigger();
  producer->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->GetSequenceNumber() == sequenceA);
  triggerA->SetValue(321);
  triggerA->Trigger();
  triggerA->Execute();
  reactor->Commit();
  auto sequenceB = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceB > sequenceA);
  CPPUNIT_ASSERT(reactor->Eval() == 321);
  reactor->Commit();
  auto sequenceC = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceC == sequenceB);
  triggerB->SetValue(314);
  triggerB->Trigger();
  triggerB->Execute();
  reactor->Commit();
  auto sequenceD = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceD > sequenceC);
  CPPUNIT_ASSERT(reactor->Eval() == 314);
  producer->SetComplete();
  producer->Trigger();
  producer->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->GetSequenceNumber() == sequenceD);
  CPPUNIT_ASSERT(!reactor->IsComplete());
  triggerA->SetComplete();
  triggerA->Trigger();
  triggerA->Execute();
  triggerB->SetComplete();
  triggerB->Trigger();
  triggerB->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->GetSequenceNumber() == sequenceD);
  CPPUNIT_ASSERT(reactor->IsComplete());
}
