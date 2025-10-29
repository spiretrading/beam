#include "Beam/Queues/StateQueue.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

using namespace Beam;

int main() {
  auto routines = RoutineHandlerGroup();
  auto receiver = std::make_shared<StateQueue<int>>();
  auto sender = std::make_shared<StateQueue<bool>>();
  routines.spawn([=] {
    while(true) {
      receiver->push(123);
      sender->pop();
    }
  });
  for(auto j = 0; j < 200; ++j) {
    routines.spawn([=] {
      while(true) {
        receiver->pop();
        sender->push(true);
      }
    });
  }
}
