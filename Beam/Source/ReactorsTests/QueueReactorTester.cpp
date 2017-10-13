#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void QueueReactorTester::TestEmptyQueue() {
  auto queue = std::make_shared<Queue<int>>();
  auto sequenceNumbers = std::make_shared<Queue<int>>();
  Trigger trigger;
  trigger.GetSequenceNumberPublisher().Monitor(sequenceNumbers);
  int dummySequenceNumber;
  for(int i = 1; i < 10; ++i) {
    trigger.SignalUpdate(Store(dummySequenceNumber));
    CPPUNIT_ASSERT(sequenceNumbers->Top() == i);
    sequenceNumbers->Pop();
  }
  auto reactor = MakeQueueReactor(static_pointer_cast<QueueReader<int>>(queue),
    Ref(trigger));
  queue->Break();
  CPPUNIT_ASSERT(sequenceNumbers->Top() == 10);
  sequenceNumbers->Pop();
  CPPUNIT_ASSERT(reactor->Commit(1) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->Commit(9) == BaseReactor::Update::NONE);
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT(reactor->Commit(10) == BaseReactor::Update::COMPLETE);
  CPPUNIT_ASSERT(reactor->Commit(11) == BaseReactor::Update::NONE);
}
