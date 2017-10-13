#include "Beam/ReactorsTests/QueueReactorTester.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace std;

void QueueReactorTester::TestEmptyQueue() {
  auto queue = std::make_shared<Queue<int>>();
  Trigger trigger;
  auto reactor = MakeQueueReactor(queue, Ref(trigger));
}
