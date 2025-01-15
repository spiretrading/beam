#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include "Beam/Queues/AggregateQueueReader.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace std::chrono_literals;

int main() {
  try {
    auto m = std::mutex();
    auto c = std::condition_variable();
    auto b = false;
    auto r1 = RoutineHandler(Spawn([&] {
      std::this_thread::sleep_for(1000ms);
      auto l = std::unique_lock(m);
      while(!b) {
        c.wait(l);
      }
    }));
    auto r2 = RoutineHandler(Spawn([&] {
      std::this_thread::sleep_for(2000ms);
      auto l = std::lock_guard(m);
      b = true;
      c.notify_all();
    }));
    r1.Wait();
    r2.Wait();
  } catch(const PipeBrokenException&) {
  }
  std::cout << "Done";
  return 0;
}
