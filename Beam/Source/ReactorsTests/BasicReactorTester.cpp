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
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  reactor->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<ReactorUnavailableException>(*reactor, 1,
    BaseReactor::Update::COMPLETE, true);
  CPPUNIT_ASSERT(sequenceNumbers->IsEmpty());
}

void BasicReactorTester::TestCompleteWithThrowImmediately() {
  Trigger trigger;
  auto reactor = MakeBasicReactor<int>(Ref(trigger));
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  reactor->SetComplete(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 1, BaseReactor::Update::EVAL, true);
  AssertException<DummyException>(*reactor, 2, BaseReactor::Update::NONE, true);
  CPPUNIT_ASSERT(sequenceNumbers->IsEmpty());
}

void BasicReactorTester::TestSingleValue() {
  Trigger trigger;
  auto reactor = MakeBasicReactor<int>(Ref(trigger));
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  reactor->Update(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, 123);
  reactor->SetComplete();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::COMPLETE, 123);
  AssertValue(*reactor, 3, BaseReactor::Update::NONE, 123, true);
}

void BasicReactorTester::TestSingleValueAndThrow() {
  Trigger trigger;
  auto reactor = MakeBasicReactor<int>(Ref(trigger));
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  AssertException<ReactorUnavailableException>(*reactor, 0,
    BaseReactor::Update::NONE);
  reactor->Update(123);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, 123);
  reactor->SetComplete(DummyException{});
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertException<DummyException>(*reactor, 2, BaseReactor::Update::EVAL, true);
  AssertException<DummyException>(*reactor, 3, BaseReactor::Update::NONE, true);
}
