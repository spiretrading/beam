#include "Beam/Queues/StateQueue.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

using namespace Beam;
using namespace Beam::Routines;

int main() {
  RoutineHandlerGroup routines;
  auto receiverQueue = std::make_shared<StateQueue<int>>();
  auto senderQueue = std::make_shared<StateQueue<bool>>();
  routines.Spawn(
    [=] {
      while(true) {
        receiverQueue->Push(123);
        senderQueue->Top();
        senderQueue->Pop();
      }
    });
  for(auto j = 0; j < 200; ++j) {
    routines.Spawn(
      [=] {
        while(true) {
          receiverQueue->Top();
          receiverQueue->Pop();
          senderQueue->Push(true);
        }
      });
  }
}
