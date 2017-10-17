#include "Beam/ReactorsTests/NonRepeatingReactorTester.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/NonRepeatingReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void NonRepeatingReactorTester::TestAAB() {
  Trigger trigger;
  auto p1 = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeNonRepeatingReactor(p1);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  p1->Update(1);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 1);
  p1->Update(1);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::NONE, 1);
  p1->Update(2);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::EVAL, 2);
}

void NonRepeatingReactorTester::TestABA() {
  Trigger trigger;
  auto p1 = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeNonRepeatingReactor(p1);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  p1->Update(1);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 1);
  p1->Update(2);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, 2);
  p1->Update(1);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::EVAL, 1);
}

void NonRepeatingReactorTester::TestBAA() {
  Trigger trigger;
  auto p1 = MakeBasicReactor<int>(Ref(trigger));
  auto reactor = MakeNonRepeatingReactor(p1);
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  p1->Update(1);
  AssertValue(*reactor, 0, BaseReactor::Update::EVAL, 1);
  p1->Update(2);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 1);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 1, BaseReactor::Update::EVAL, 2);
  p1->Update(2);
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 2);
  sequenceNumbers->Pop();
  AssertValue(*reactor, 2, BaseReactor::Update::NONE, 2);
}