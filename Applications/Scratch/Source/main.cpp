#include <iostream>
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace std::chrono_literals;

int main() {
  auto m = std::mutex();
  auto c = std::condition_variable();
  auto b = false;
  auto r = RoutineHandler(Spawn([&] {
    auto l = std::unique_lock(m);
    while(!b) {
      if(c.wait_for(l, 3s) == std::cv_status::timeout) {
        std::cout << "Timedout" << std::endl;
      }
    }
  }));
  std::this_thread::sleep_for(9s);
  {
    auto l = std::lock_guard(m);
    b = true;
    c.notify_all();
  }
  r.Wait();
  std::cout << "Done" << std::endl;
  return 0;
}
