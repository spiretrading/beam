#include <array>
#include <condition_variable>
#include <thread>
#include <doctest/doctest.h>
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;
using namespace std::chrono;

TEST_SUITE("ConditionVariable") {
  TEST_CASE_TEMPLATE("same_context_wait", T, std::condition_variable,
      std::condition_variable_any) {
    auto condition = T();
    auto mutex = std::mutex();
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
      value = 1;
      condition.notify_one();
      defer();
      value = 2;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto received = result.get();
    wait(consumer);
    wait(producer);
    REQUIRE(received == 2);
  }

  TEST_CASE_TEMPLATE("native_wait_mutex_handoff", T, std::condition_variable,
      std::condition_variable_any) {
    auto condition = T();
    auto mutex = std::mutex();
    auto lock = std::unique_lock(mutex);
    auto ready = Async<void>();
    auto value = 0;
    auto producer = spawn([&] {
      auto lock = std::lock_guard(mutex);
      value = 42;
      condition.notify_one();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto witness = spawn([&] {
      ready.get_eval().set();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    ready.get();
    while(value == 0) {
      condition.wait(lock);
    }
    REQUIRE(value == 42);
    lock.unlock();
    wait(producer);
    wait(witness);
  }

  TEST_CASE_TEMPLATE("native_timed_wait", T, std::condition_variable,
      std::condition_variable_any) {
    auto condition = T();
    auto mutex = std::mutex();
    auto lock = std::unique_lock(mutex);
    auto ready = Async<void>();
    auto value = 0;
    auto producer = spawn([&] {
      auto lock = std::lock_guard(mutex);
      value = 42;
      condition.notify_one();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto witness = spawn([&] {
      ready.get_eval().set();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    ready.get();
    auto received = condition.wait_for(lock, seconds(5), [&] {
      return value != 0;
    });
    REQUIRE(received);
    REQUIRE(value == 42);
    auto timeout = condition.wait_for(lock, milliseconds(1), [] {
      return false;
    });
    REQUIRE(!timeout);
    REQUIRE(lock.owns_lock());
    lock.unlock();
    wait(producer);
    wait(witness);
  }

  TEST_CASE_TEMPLATE("routine_timed_wait", T, std::condition_variable,
      std::condition_variable_any) {
    auto condition = T();
    auto mutex = std::mutex();
    auto ready = Async<void>();
    auto result = Async<bool>();
    auto value = 0;
    auto consumer = spawn([&] {
      auto lock = std::unique_lock(mutex);
      ready.get_eval().set();
      auto received = condition.wait_for(lock, seconds(5), [&] {
        return value == 42;
      });
      auto timed_out = !condition.wait_for(lock, milliseconds(1), [] {
        return false;
      });
      return received && timed_out && lock.owns_lock();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    ready.get();
    auto producer = std::thread([&] {
      auto lock = std::lock_guard(mutex);
      value = 42;
      condition.notify_one();
    });
    producer.join();
    auto received = result.get();
    wait(consumer);
    REQUIRE(received);
  }

  TEST_CASE_TEMPLATE("mixed_notify_all", T, std::condition_variable,
      std::condition_variable_any) {
    auto condition = T();
    auto mutex = std::mutex();
    auto ready = Async<void>();
    auto count = 0;
    auto finished = 0;
    auto released = false;
    auto consumer = [&] {
      auto lock = std::unique_lock(mutex);
      if(++count == 6) {
        ready.get_eval().set();
      }
      while(!released) {
        condition.wait(lock);
      }
      ++finished;
    };
    auto results = std::array<Async<void>, 4>();
    auto routines = std::array<Routine::Id, 4>();
    auto contexts = std::min(std::size_t(2),
      Details::Scheduler::get().get_thread_count());
    for(auto i = std::size_t(0); i != routines.size(); ++i) {
      routines[i] = spawn(consumer, Details::Scheduler::DEFAULT_STACK_SIZE,
        i % contexts, results[i].get_eval());
    }
    auto first = std::thread(consumer);
    auto second = std::thread(consumer);
    ready.get();
    {
      auto lock = std::lock_guard(mutex);
      released = true;
      condition.notify_all();
    }
    first.join();
    second.join();
    for(auto& result : results) {
      result.get();
    }
    for(auto routine : routines) {
      wait(routine);
    }
    REQUIRE(finished == 6);
  }

  TEST_CASE_TEMPLATE("worker_handoff", T, std::condition_variable,
      std::condition_variable_any) {
    auto condition = T();
    auto mutex = std::mutex();
    auto turn = 0;
    auto count = 0;
    auto results = std::array<Async<void>, 2>();
    auto routines = std::array<Routine::Id, 2>();
    auto contexts = std::min(std::size_t(2),
      Details::Scheduler::get().get_thread_count());
    for(auto i = 0; i != 2; ++i) {
      routines[i] = spawn([&, i] {
        auto lock = std::unique_lock(mutex);
        for(auto j = 0; j != 200; ++j) {
          while(turn != i) {
            condition.wait(lock);
          }
          ++count;
          turn = 1 - i;
          condition.notify_one();
        }
      }, Details::Scheduler::DEFAULT_STACK_SIZE, i % contexts,
        results[i].get_eval());
    }
    for(auto& result : results) {
      result.get();
    }
    for(auto routine : routines) {
      wait(routine);
    }
    REQUIRE(count == 400);
  }

  TEST_CASE("stop_token_wait") {
    auto condition = std::condition_variable_any();
    auto mutex = std::mutex();
    auto source = std::stop_source();
    auto ready = Async<void>();
    auto result = Async<bool>();
    auto consumer = spawn([&] {
      auto lock = std::unique_lock(mutex);
      ready.get_eval().set();
      return condition.wait(lock, source.get_token(), [] {
        return false;
      });
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    ready.get();
    auto producer = spawn([&] {
      source.request_stop();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto received = result.get();
    wait(consumer);
    wait(producer);
    REQUIRE(!received);
  }

  TEST_CASE("thread_exit_notification") {
    auto condition = std::condition_variable();
    auto mutex = std::mutex();
    auto ready = Async<void>();
    auto result = Async<int>();
    auto value = 0;
    auto consumer = spawn([&] {
      auto lock = std::unique_lock(mutex);
      ready.get_eval().set();
      condition.wait(lock, [&] { return value != 0; });
      return value;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    ready.get();
    auto producer = std::thread([&] {
      auto lock = std::unique_lock(mutex);
      value = 42;
      std::notify_all_at_thread_exit(condition, std::move(lock));
    });
    producer.join();
    auto received = result.get();
    wait(consumer);
    REQUIRE(received == 42);
  }
}
