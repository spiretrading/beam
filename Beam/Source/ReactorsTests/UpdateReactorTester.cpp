#include "Beam/ReactorsTests/UpdateReactorTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/NoneReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Reactors/UpdateReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void UpdateReactorTester::TestImmediateCompletion() {
  auto reactor = MakeUpdateReactor(MakeNoneReactor<int>());
  AssertValue(*reactor, 0, BaseReactor::Update::COMPLETE_WITH_EVAL,
    BaseReactor::Update::COMPLETE);
}

void UpdateReactorTester::TestCompleteWithEvaluation() {
  auto reactor = MakeUpdateReactor(MakeConstantReactor(123));
  AssertValue(*reactor, 0, BaseReactor::Update::COMPLETE_WITH_EVAL,
    BaseReactor::Update::COMPLETE_WITH_EVAL);
}

void UpdateReactorTester::TestEvaluationThenCompletion() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto child = MakeBasicReactor<int>();
  auto reactor = MakeUpdateReactor(child);
  AssertValue(*reactor, 0, BaseReactor::Update::NONE,
    BaseReactor::Update::NONE);
  child->Update(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL,
    BaseReactor::Update::EVAL);
  child->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::COMPLETE_WITH_EVAL,
    BaseReactor::Update::COMPLETE);
}

void UpdateReactorTester::TestEvaluationThenNoneThenCompletion() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto child = MakeBasicReactor<int>();
  auto reactor = MakeUpdateReactor(child);
  AssertValue(*reactor, 0, BaseReactor::Update::NONE,
    BaseReactor::Update::NONE);
  child->Update(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL,
    BaseReactor::Update::EVAL);
  int dummySequenceNumber;
  trigger.SignalUpdate(Store(dummySequenceNumber));
  CPPUNIT_ASSERT(dummySequenceNumber == 2);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == dummySequenceNumber);
  sequenceNumbers->Pop();
  AssertValue(*reactor, dummySequenceNumber, BaseReactor::Update::EVAL,
    BaseReactor::Update::NONE);
  child->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 3);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 3, BaseReactor::Update::COMPLETE_WITH_EVAL,
    BaseReactor::Update::COMPLETE);
}

void UpdateReactorTester::TestEvaluationThenNoneThenNoneThenCompletion() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto child = MakeBasicReactor<int>();
  auto reactor = MakeUpdateReactor(child);
  AssertValue(*reactor, 0, BaseReactor::Update::NONE,
    BaseReactor::Update::NONE);
  child->Update(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL,
    BaseReactor::Update::EVAL);
  int dummySequenceNumber;
  trigger.SignalUpdate(Store(dummySequenceNumber));
  CPPUNIT_ASSERT(dummySequenceNumber == 2);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == dummySequenceNumber);
  sequenceNumbers->Pop();
  AssertValue(*reactor, dummySequenceNumber, BaseReactor::Update::EVAL,
    BaseReactor::Update::NONE);
  trigger.SignalUpdate(Store(dummySequenceNumber));
  CPPUNIT_ASSERT(dummySequenceNumber == 3);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == dummySequenceNumber);
  sequenceNumbers->Pop();
  AssertValue(*reactor, dummySequenceNumber, BaseReactor::Update::NONE,
    BaseReactor::Update::NONE);
  child->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 4);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 4, BaseReactor::Update::COMPLETE_WITH_EVAL,
    BaseReactor::Update::COMPLETE);
}
