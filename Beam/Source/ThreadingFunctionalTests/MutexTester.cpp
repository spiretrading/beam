#include <array>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <doctest/doctest.h>
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;

TEST_SUITE("Mutex") {
  TEST_CASE_TEMPLATE("fifo_handoff", T, std::mutex, std::recursive_mutex) {
    auto mutex = T();
    auto acquired = Async<void>();
    auto release = Async<void>();
    auto results = std::array<Async<int>, 3>();
    auto routines = std::array<Routine::Id, 3>();
    auto count = 0;
    auto barged = false;
    routines[2] = spawn([&] {
      auto lock = std::unique_lock(mutex);
      acquired.get_eval().set();
      release.get();
      if constexpr(std::is_same_v<T, std::recursive_mutex>) {
        mutex.lock();
        mutex.unlock();
      }
      lock.unlock();
      barged = mutex.try_lock();
      if(barged) {
        mutex.unlock();
      }
      lock.lock();
      return count++;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, results[2].get_eval());
    acquired.get();
    for(auto i = 0; i != 2; ++i) {
      routines[i] = spawn([&] {
        auto lock = std::lock_guard(mutex);
        return count++;
      }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, results[i].get_eval());
    }
    auto releaser = spawn([&] {
      release.get_eval().set();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto order = std::array<int, 3>();
    for(auto i = 0; i != 3; ++i) {
      order[i] = results[i].get();
      wait(routines[i]);
    }
    wait(releaser);
    REQUIRE(!barged);
    REQUIRE(order == std::array{0, 1, 2});
  }

  TEST_CASE_TEMPLATE("native_handoff_reservation", T, std::mutex,
      std::recursive_mutex) {
    auto mutex = T();
    auto lock = std::unique_lock(mutex);
    auto ready = Async<void>();
    auto release = std::atomic_bool(false);
    auto result = Async<void>();
    auto waiter = spawn([&] {
      auto lock = std::lock_guard(mutex);
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    auto witness = spawn([&] {
      ready.get_eval().set();
      release.wait(false);
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    ready.get();
    lock.unlock();
    auto barged = mutex.try_lock();
    if(barged) {
      mutex.unlock();
    }
    release.store(true);
    release.notify_one();
    result.get();
    wait(waiter);
    wait(witness);
    REQUIRE(!barged);
  }

  TEST_CASE_TEMPLATE("same_context_contention", T, std::mutex,
      std::recursive_mutex) {
    auto mutex = T();
    auto acquired = Async<void>();
    auto release = Async<void>();
    auto owner_result = Async<void>();
    auto result = Async<bool>();
    auto observed = Async<bool>();
    auto owner = spawn([&] {
      auto lock = std::lock_guard(mutex);
      acquired.get_eval().set();
      release.get();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, owner_result.get_eval());
    acquired.get();
    auto waiter = spawn([&] {
      auto is_acquired = mutex.try_lock();
      if(is_acquired) {
        mutex.unlock();
      }
      auto lock = std::lock_guard(mutex);
      return !is_acquired;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    auto releaser = spawn([&] {
      auto is_pending = result.get_state() == BaseAsync::State::PENDING;
      release.get_eval().set();
      return is_pending;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, observed.get_eval());
    auto is_exclusive = result.get();
    auto is_suspended = observed.get();
    owner_result.get();
    wait(owner);
    wait(waiter);
    wait(releaser);
    REQUIRE(is_exclusive);
    REQUIRE(is_suspended);
  }

  TEST_CASE("recursive_depth") {
    auto mutex = std::recursive_mutex();
    auto acquired = Async<void>();
    auto release = Async<void>();
    auto remaining = Async<void>();
    auto finish = Async<void>();
    auto result = Async<bool>();
    auto owner = spawn([&] {
      auto lock = std::unique_lock(mutex);
      auto nested_lock = std::unique_lock(mutex);
      auto nested = mutex.try_lock();
      acquired.get_eval().set();
      release.get();
      if(nested) {
        mutex.unlock();
      }
      nested_lock.unlock();
      remaining.get_eval().set();
      finish.get();
      return nested;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    acquired.get();
    auto probe = Async<bool>();
    auto contender = spawn([&] {
      auto first = mutex.try_lock();
      if(first) {
        mutex.unlock();
      }
      release.get_eval().set();
      remaining.get();
      auto second = mutex.try_lock();
      if(second) {
        mutex.unlock();
      }
      finish.get_eval().set();
      auto lock = std::lock_guard(mutex);
      return !first && !second;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, probe.get_eval());
    auto is_recursive = result.get();
    auto is_exclusive = probe.get();
    wait(owner);
    wait(contender);
    REQUIRE(is_recursive);
    REQUIRE(is_exclusive);
  }

  TEST_CASE_TEMPLATE("native_thread_contention", T, std::mutex,
      std::recursive_mutex) {
    auto mutex = T();
    auto lock = std::unique_lock(mutex);
    auto result = Async<bool>();
    auto ready = Async<void>();
    auto waiter = spawn([&] {
      auto acquired = mutex.try_lock();
      if(acquired) {
        mutex.unlock();
      }
      auto lock = std::lock_guard(mutex);
      return !acquired;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    auto witness = spawn([&] {
      ready.get_eval().set();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    ready.get();
    lock.unlock();
    auto is_exclusive = result.get();
    wait(waiter);
    wait(witness);
    REQUIRE(is_exclusive);
    auto acquired = Async<void>();
    auto release = Async<void>();
    auto owner_result = Async<void>();
    auto owner = spawn([&] {
      auto lock = std::lock_guard(mutex);
      acquired.get_eval().set();
      release.get();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, owner_result.get_eval());
    acquired.get();
    auto native_ready = Async<void>();
    auto is_native_exclusive = false;
    auto is_native_acquired = false;
    auto thread = std::thread([&] {
      auto acquired = mutex.try_lock();
      if(acquired) {
        mutex.unlock();
      }
      is_native_exclusive = !acquired;
      native_ready.get_eval().set();
      auto lock = std::lock_guard(mutex);
      is_native_acquired = true;
    });
    native_ready.get();
    release.get_eval().set();
    thread.join();
    owner_result.get();
    wait(owner);
    REQUIRE(is_native_exclusive);
    REQUIRE(is_native_acquired);
  }

  TEST_CASE_TEMPLATE("worker_contention", T, std::mutex,
      std::recursive_mutex) {
    auto mutex = T();
    auto count = 0;
    auto results = std::array<Async<void>, 8>();
    auto routines = std::array<Routine::Id, 8>();
    auto contexts = std::min(std::size_t(2),
      Details::Scheduler::get().get_thread_count());
    for(auto i = std::size_t(0); i != results.size(); ++i) {
      routines[i] = spawn([&] {
        for(auto j = 0; j != 100; ++j) {
          auto lock = std::lock_guard(mutex);
          auto next = count + 1;
          defer();
          count = next;
        }
      }, Details::Scheduler::DEFAULT_STACK_SIZE, i % contexts,
        results[i].get_eval());
    }
    auto native = std::thread([&] {
      for(auto i = 0; i != 100; ++i) {
        auto lock = std::lock_guard(mutex);
        ++count;
      }
    });
    native.join();
    for(auto& result : results) {
      result.get();
    }
    for(auto routine : routines) {
      wait(routine);
    }
    REQUIRE(count == 900);
  }

  TEST_CASE_TEMPLATE("condition_variable_wait", T, std::mutex,
      std::recursive_mutex) {
    auto mutex = T();
    auto condition = std::condition_variable_any();
    auto ready = Async<void>();
    auto result = Async<int>();
    auto value = 0;
    auto consumer = spawn([&] {
      auto lock = std::unique_lock(mutex);
      ready.get_eval().set();
      while(value == 0) {
        condition.wait(lock);
      }
      return value;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    ready.get();
    auto producer = spawn([&] {
      auto lock = std::lock_guard(mutex);
      value = 42;
      condition.notify_one();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto received = result.get();
    wait(consumer);
    wait(producer);
    REQUIRE(received == 42);
  }
}
