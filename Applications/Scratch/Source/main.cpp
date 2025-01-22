#include <iostream>
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace std::chrono_literals;

int main() {
  auto m1 = std::mutex();
  auto m2 = std::mutex();
  m1.lock();
  m2.lock();
  auto r1 = RoutineHandler(Spawn([&] {
    m1.lock();
  }));
  auto r2 = RoutineHandler(Spawn([&] {
    m2.lock();
  }));
  std::this_thread::sleep_for(5s);
  auto r3 = RoutineHandler(Spawn([&] {
    std::cout << "Yo";
  }));
  std::this_thread::sleep_for(5s);
  m1.unlock();
  m2.unlock();
  r1.Wait();
  r2.Wait();
  r3.Wait();
  std::cout << "Done" << std::endl;
  return 0;
}
