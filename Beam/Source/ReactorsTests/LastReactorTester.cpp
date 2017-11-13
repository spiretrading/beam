#include "Beam/ReactorsTests/LastReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/ChainReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/LastReactor.hpp"
#include "Beam/Reactors/NoneReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void LastReactorTester::TestImmediateCompletion() {
  auto reactor = MakeLastReactor(MakeNoneReactor<int>());
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::COMPLETE);
}

void LastReactorTester::TestCompleteWithEvaluation() {
  auto reactor = MakeLastReactor(MakeConstantReactor(543));
  AssertValue(*reactor, 0, BaseReactor::Update::COMPLETE_WITH_EVAL, 543);
}

void LastReactorTester::TestEvaluationThenCompleteWithEvaluation() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto reactor = MakeLastReactor(MakeChainReactor(123, 456));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::COMPLETE_WITH_EVAL, 456);
}

void LastReactorTester::TestEvaluationThenEvaluationThenComplete() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto reactor = MakeLastReactor(MakeChainReactor(123,
    MakeChainReactor(456, MakeNoneReactor<int>())));
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::COMPLETE_WITH_EVAL, 456);
}
