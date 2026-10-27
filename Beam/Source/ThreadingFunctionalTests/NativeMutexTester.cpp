#include <array>
#include <mutex>
#include <thread>
#include <doctest/doctest.h>
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Threading/NativeMutex.hpp"

using namespace Beam;

namespace {
  void test_contention(NativeMutex& mutex) {
    auto lock = std::unique_lock(mutex);
    auto is_acquired = true;
    auto count = 0;
    auto contender = std::thread([&] {
      is_acquired = mutex.try_lock();
      if(is_acquired) {
        mutex.unlock();
      }
    });
    contender.join();
    REQUIRE(!is_acquired);
    lock.unlock();
    auto threads = std::array<std::thread, 4>();
    for(auto& thread : threads) {
      thread = std::thread([&] {
        for(auto i = 0; i != 1000; ++i) {
          auto lock = std::lock_guard(mutex);
          ++count;
        }
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    REQUIRE(count == 4000);
    is_acquired = mutex.try_lock();
    if(is_acquired) {
      mutex.unlock();
    }
    REQUIRE(is_acquired);
  }
}

TEST_SUITE("NativeMutex") {
  TEST_CASE("hook_initialization") {
    auto mutex = NativeMutex();
    test_contention(mutex);
    Details::Scheduler::get();
    test_contention(mutex);
    auto initialized = NativeMutex();
    test_contention(initialized);
  }

  TEST_CASE("routine_contention") {
    Details::Scheduler::get();
    auto mutex = NativeMutex();
    auto result = Async<void>();
    auto lock = std::unique_lock(mutex);
    auto count = 0;
    auto routine = spawn([&] {
      for(auto i = 0; i != 1000; ++i) {
        auto lock = std::lock_guard(mutex);
        ++count;
      }
    }, result.get_eval());
    lock.unlock();
    for(auto i = 0; i != 1000; ++i) {
      auto lock = std::lock_guard(mutex);
      ++count;
    }
    result.get();
    wait(routine);
    REQUIRE(count == 2000);
  }
}
