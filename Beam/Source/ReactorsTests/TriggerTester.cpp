#include "Beam/ReactorsTests/TriggerTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/Trigger.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void TriggerTester::TestSignalUpdate() {
  Trigger trigger;
  auto queue = std::make_shared<Queue<int>>();
  trigger.GetSequenceNumberPublisher().Monitor(queue);
  int sequenceNumber;
  trigger.SignalUpdate(Store(sequenceNumber));
  CPPUNIT_ASSERT(sequenceNumber == 1);
  CPPUNIT_ASSERT(queue->Top() == sequenceNumber);
  queue->Pop();
  trigger.SignalUpdate(Store(sequenceNumber));
  CPPUNIT_ASSERT(sequenceNumber == 2);
  CPPUNIT_ASSERT(queue->Top() == sequenceNumber);
}
