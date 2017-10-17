#include "Beam/ReactorsTests/BasicReactorTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void BasicReactorTester::TestCompleteImmediately() {
  Trigger trigger;
  auto reactor = MakeBasicReactor<int>(Ref(trigger));
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  reactor->SetComplete();
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::COMPLETE);
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::NONE, true);
  CPPUNIT_ASSERT(sequenceNumbers->IsEmpty());
}

void BasicReactorTester::TestCompleteWithThrowImmediately() {
  Trigger trigger;
  auto reactor = MakeBasicReactor<int>(Ref(trigger));
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  reactor->SetComplete(DummyException{});
  AssertException<DummyException>(*reactor, 0, BaseReactor::Update::EVAL, true);
  AssertException<DummyException>(*reactor, 1, BaseReactor::Update::NONE, true);
  CPPUNIT_ASSERT(sequenceNumbers->IsEmpty());
}
