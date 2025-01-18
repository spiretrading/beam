#include <iostream>
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace std::chrono_literals;

int main() {
  auto r = RoutineHandler(Spawn([=] {
    std::this_thread::sleep_for(10s);
  }));
  r.Wait();
  std::cout << "Done";
  return 0;
}
