#include <array>
#include <atomic>
#include <thread>
#include <doctest/doctest.h>
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Threading/NativeConditionVariable.hpp"

using namespace Beam;
using namespace std::chrono;

namespace {
  void test_notifications(NativeMutex& mutex,
      NativeConditionVariable& condition) {
    auto is_ready = false;
    auto is_locked = false;
    auto value = 0;
    auto thread = std::thread([&] {
      auto lock = std::unique_lock(mutex);
      condition.wait(lock, [&] { return is_ready; });
      is_locked = lock.owns_lock();
      value = 42;
    });
    {
      auto lock = std::lock_guard(mutex);
      is_ready = true;
    }
    condition.notify_one();
    thread.join();
    REQUIRE(is_locked);
    REQUIRE(value == 42);
    auto threads = std::array<std::thread, 4>();
    auto ready = NativeConditionVariable();
    auto count = 0;
    is_ready = false;
    for(auto& thread : threads) {
      thread = std::thread([&] {
        auto lock = std::unique_lock(mutex);
        ++count;
        ready.notify_one();
        while(!is_ready) {
          condition.wait(lock);
        }
        ++value;
      });
    }
    {
      auto lock = std::unique_lock(mutex);
      ready.wait(lock, [&] { return count == 4; });
      is_ready = true;
    }
    condition.notify_all();
    for(auto& thread : threads) {
      thread.join();
    }
    REQUIRE(value == 46);
  }

  void test_timeouts(NativeMutex& mutex, NativeConditionVariable& condition) {
    auto lock = std::unique_lock(mutex);
    REQUIRE(condition.wait_for(lock, nanoseconds::zero()) ==
      std::cv_status::timeout);
    REQUIRE(condition.wait_for(lock, hours::min()) == std::cv_status::timeout);
    REQUIRE(condition.wait_until(lock, steady_clock::now() - seconds(1)) ==
      std::cv_status::timeout);
    REQUIRE(condition.wait_until(lock, system_clock::now() - seconds(1)) ==
      std::cv_status::timeout);
    REQUIRE(condition.wait_for(lock, milliseconds(-1), [] { return true; }));
    REQUIRE(condition.wait_until(lock, steady_clock::now() - seconds(1),
      [] { return true; }));
    auto started = steady_clock::now();
    auto timeout = duration<double, std::milli>(2.5);
    REQUIRE(!condition.wait_for(lock, timeout, [] { return false; }));
    REQUIRE(steady_clock::now() - started >= timeout);
    auto deadline = system_clock::now() + milliseconds(2);
    REQUIRE(!condition.wait_until(lock, deadline, [] { return false; }));
    REQUIRE(system_clock::now() >= deadline);
    REQUIRE(lock.owns_lock());
  }

  void test_timed_notification(auto receive) {
    auto mutex = NativeMutex();
    auto condition = NativeConditionVariable();
    auto ready = NativeConditionVariable();
    auto is_waiting = false;
    auto is_ready = false;
    auto is_notified = false;
    auto is_locked = false;
    auto thread = std::thread([&] {
      auto lock = std::unique_lock(mutex);
      is_waiting = true;
      ready.notify_one();
      is_notified = receive(condition, lock, [&] { return is_ready; });
      is_locked = lock.owns_lock();
    });
    {
      auto lock = std::unique_lock(mutex);
      ready.wait(lock, [&] { return is_waiting; });
      is_ready = true;
    }
    condition.notify_one();
    thread.join();
    REQUIRE(is_notified);
    REQUIRE(is_locked);
  }
}

TEST_SUITE("NativeConditionVariable") {
  TEST_CASE("hook_initialization") {
    auto mutex = NativeMutex();
    auto condition = NativeConditionVariable();
    test_notifications(mutex, condition);
    Details::Scheduler::get();
    test_notifications(mutex, condition);
    auto initialized = NativeConditionVariable();
    test_notifications(mutex, initialized);
  }

  TEST_CASE("routine_wait") {
    auto is_timed = false;
    SUBCASE("untimed") {}
    SUBCASE("timed") {
      is_timed = true;
    }
    Details::Scheduler::get();
    auto mutex = NativeMutex();
    auto condition = NativeConditionVariable();
    auto ready = NativeConditionVariable();
    auto is_waiting = false;
    auto is_ready = false;
    auto result = Async<bool>();
    auto routine = spawn([&] {
      auto lock = std::unique_lock(mutex);
      is_waiting = true;
      ready.notify_one();
      if(is_timed) {
        auto notified = condition.wait_for(
          lock, seconds(5), [&] { return is_ready; });
        return notified && lock.owns_lock();
      }
      condition.wait(lock, [&] { return is_ready; });
      return lock.owns_lock();
    }, result.get_eval());
    {
      auto lock = std::unique_lock(mutex);
      ready.wait(lock, [&] { return is_waiting; });
      is_ready = true;
    }
    condition.notify_one();
    auto is_locked = result.get();
    wait(routine);
    REQUIRE(is_locked);
  }

  TEST_CASE("timed_waits") {
    auto mutex = NativeMutex();
    auto condition = NativeConditionVariable();
    test_timeouts(mutex, condition);
    Details::Scheduler::get();
    test_timeouts(mutex, condition);
  }

  TEST_CASE("timed_notifications") {
    Details::Scheduler::get();
    SUBCASE("relative") {
      test_timed_notification([] (auto& condition, auto& lock, auto predicate) {
        return condition.wait_for(lock, seconds(5), predicate);
      });
    }
    SUBCASE("steady_deadline") {
      test_timed_notification([] (auto& condition, auto& lock, auto predicate) {
        auto deadline = steady_clock::now() + seconds(5);
        while(!predicate()) {
          if(condition.wait_until(lock, deadline) == std::cv_status::timeout) {
            return false;
          }
        }
        return true;
      });
    }
    SUBCASE("system_deadline") {
      test_timed_notification([] (auto& condition, auto& lock, auto predicate) {
        return condition.wait_until(
          lock, system_clock::now() + seconds(5), predicate);
      });
    }
    SUBCASE("large_duration") {
      test_timed_notification([] (auto& condition, auto& lock, auto predicate) {
        return condition.wait_for(lock, hours::max(), predicate);
      });
    }
  }

  TEST_CASE("notifications_preserve_deadline") {
    Details::Scheduler::get();
    auto mutex = NativeMutex();
    auto condition = NativeConditionVariable();
    auto is_started = std::atomic_bool(false);
    auto is_finished = std::atomic_bool(false);
    auto notifier = std::thread([&] {
      is_started.wait(false);
      while(!is_finished.load()) {
        condition.notify_one();
        std::this_thread::yield();
      }
    });
    auto lock = std::unique_lock(mutex);
    auto started = steady_clock::now();
    is_started.store(true);
    is_started.notify_one();
    auto result = condition.wait_for(
      lock, milliseconds(20), [] { return false; });
    auto elapsed = steady_clock::now() - started;
    is_finished.store(true);
    notifier.join();
    REQUIRE(!result);
    REQUIRE(elapsed >= milliseconds(20));
    REQUIRE(lock.owns_lock());
  }
}
