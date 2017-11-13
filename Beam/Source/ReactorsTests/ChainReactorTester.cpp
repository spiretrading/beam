#include "Beam/ReactorsTests/ChainReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/ChainReactor.hpp"
#include "Beam/Reactors/NoneReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void ChainReactorTester::TestConstantChain() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto reactor = Chain(100, 200);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 100);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::COMPLETE_WITH_EVAL, 200);
}

void ChainReactorTester::TestSingleValue() {
  Trigger trigger;
  Trigger::SetEnvironmentTrigger(trigger);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  auto reactor = Chain(911, None<int>());
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 911);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::COMPLETE, 911);
}
