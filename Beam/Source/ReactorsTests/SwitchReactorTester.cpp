#include "Beam/ReactorsTests/SwitchReactorTester.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/NoneReactor.hpp"
#include "Beam/Reactors/SwitchReactor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;

void SwitchReactorTester::TestNoneProducer() {
  auto reactor = MakeSwitchReactor(MakeNoneReactor<ConstantReactor<int>*>());
  auto sequence = reactor->GetSequenceNumber();
  reactor->Commit();
  CPPUNIT_ASSERT_EQUAL(sequence, reactor->GetSequenceNumber());
  CPPUNIT_ASSERT(reactor->IsComplete());
}

void SwitchReactorTester::TestProducerExceptionCompletion() {
  {
    auto trigger = MakeTriggeredReactor<std::shared_ptr<ConstantReactor<int>>>();
    auto reactor = MakeSwitchReactor(trigger);
    trigger->SetException(ReactorUnavailableException());
    trigger->SetComplete();
    trigger->Trigger();
    trigger->Execute();
    reactor->Commit();
    CPPUNIT_ASSERT(reactor->HasEvaluation());
    CPPUNIT_ASSERT(reactor->IsComplete());
    CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
  }
  {
    auto trigger = MakeTriggeredReactor<
      std::shared_ptr<ConstantReactor<int>>>();
    auto reactor = MakeSwitchReactor(trigger);
    trigger->SetException(ReactorUnavailableException());
    trigger->Trigger();
    trigger->Execute();
    reactor->Commit();
    CPPUNIT_ASSERT(reactor->HasEvaluation());
    CPPUNIT_ASSERT_THROW(reactor->Eval(), ReactorUnavailableException);
    CPPUNIT_ASSERT(!reactor->IsComplete());
    trigger->SetComplete();
    trigger->Trigger();
    trigger->Execute();
    reactor->Commit();
    CPPUNIT_ASSERT(reactor->IsComplete());
  }
}
