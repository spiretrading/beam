#include <algorithm>
#include <array>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <doctest/doctest.h>
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;

TEST_SUITE("CallOnce") {
  TEST_CASE("same_context_contention") {
    auto once = std::once_flag();
    auto started = Async<void>();
    auto release = Async<void>();
    auto owner_result = Async<void>();
    auto results = std::array<Async<int>, 4>();
    auto waiters = std::array<Routine::Id, 4>();
    auto observed = Async<bool>();
    auto count = 0;
    auto value = 0;
    auto owner = spawn([&] {
      std::call_once(once, [&] {
        ++count;
        started.get_eval().set();
        release.get();
        value = 42;
      });
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, owner_result.get_eval());
    started.get();
    for(auto i = std::size_t(0); i != waiters.size(); ++i) {
      waiters[i] = spawn([&] {
        std::call_once(once, [&] {
          ++count;
          value = -1;
        });
        return value;
      }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, results[i].get_eval());
    }
    auto witness = spawn([&] {
      auto is_pending = std::ranges::all_of(results, [] (const auto& result) {
        return result.get_state() == BaseAsync::State::PENDING;
      });
      release.get_eval().set();
      return is_pending;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, observed.get_eval());
    auto is_suspended = observed.get();
    for(auto& result : results) {
      REQUIRE(result.get() == 42);
    }
    owner_result.get();
    wait(owner);
    wait(witness);
    for(auto waiter : waiters) {
      wait(waiter);
    }
    REQUIRE(is_suspended);
    REQUIRE(count == 1);
  }

  TEST_CASE("exception_retry") {
    auto once = std::once_flag();
    auto started = Async<void>();
    auto release = Async<void>();
    auto owner_result = Async<bool>();
    auto results = std::array<Async<int>, 4>();
    auto waiters = std::array<Routine::Id, 4>();
    auto count = 0;
    auto value = 0;
    auto owner = spawn([&] {
      try {
        std::call_once(once, [&] {
          ++count;
          started.get_eval().set();
          release.get();
          throw std::runtime_error("initialization failed");
        });
      } catch(const std::runtime_error&) {
        return true;
      }
      return false;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, owner_result.get_eval());
    started.get();
    for(auto i = std::size_t(0); i != waiters.size(); ++i) {
      waiters[i] = spawn([&] {
        std::call_once(once, [&] {
          ++count;
          defer();
          value = 42;
        });
        return value;
      }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, results[i].get_eval());
    }
    auto releaser = spawn([&] {
      release.get_eval().set();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto is_exception = owner_result.get();
    for(auto& result : results) {
      REQUIRE(result.get() == 42);
    }
    wait(owner);
    wait(releaser);
    for(auto waiter : waiters) {
      wait(waiter);
    }
    REQUIRE(is_exception);
    REQUIRE(count == 2);
  }

  TEST_CASE("native_initializer") {
    auto should_throw = false;
    SUBCASE("completion") {}
    SUBCASE("exception") {
      should_throw = true;
    }
    Details::Scheduler::get();
    auto once = std::once_flag();
    auto started = Async<void>();
    auto release = Async<void>();
    auto result = Async<int>();
    auto observed = Async<bool>();
    auto count = 0;
    auto value = 0;
    auto is_exception = false;
    auto owner = std::thread([&] {
      try {
        std::call_once(once, [&] {
          ++count;
          started.get_eval().set();
          release.get();
          if(should_throw) {
            throw std::runtime_error("initialization failed");
          }
          value = 42;
        });
      } catch(const std::runtime_error&) {
        is_exception = true;
      }
    });
    started.get();
    auto waiter = spawn([&] {
      std::call_once(once, [&] {
        ++count;
        value = 42;
      });
      return value;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    auto witness = spawn([&] {
      auto is_pending = result.get_state() == BaseAsync::State::PENDING;
      release.get_eval().set();
      return is_pending;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, observed.get_eval());
    auto is_suspended = observed.get();
    auto received = result.get();
    owner.join();
    wait(waiter);
    wait(witness);
    REQUIRE(received == 42);
    REQUIRE(is_suspended);
    REQUIRE(is_exception == should_throw);
    REQUIRE(count == 1 + static_cast<int>(should_throw));
  }

  TEST_CASE("mixed_waiters") {
    auto should_throw = false;
    SUBCASE("completion") {}
    SUBCASE("exception") {
      should_throw = true;
    }
    auto once = std::once_flag();
    auto started = Async<void>();
    auto release = Async<void>();
    auto owner_result = Async<bool>();
    auto results = std::array<Async<int>, 8>();
    auto waiters = std::array<Routine::Id, 8>();
    auto ready = std::array<Async<void>, 2>();
    auto native_results = std::array<int, 2>();
    auto threads = std::array<std::thread, 2>();
    auto contexts = std::min(std::size_t(2),
      Details::Scheduler::get().get_thread_count());
    auto count = 0;
    auto value = 0;
    auto owner = spawn([&] {
      try {
        std::call_once(once, [&] {
          ++count;
          started.get_eval().set();
          release.get();
          if(should_throw) {
            throw std::runtime_error("initialization failed");
          }
          value = 42;
        });
      } catch(const std::runtime_error&) {
        return true;
      }
      return false;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, owner_result.get_eval());
    started.get();
    auto consume = [&] {
      std::call_once(once, [&] {
        ++count;
        defer();
        value = 42;
      });
      return value;
    };
    for(auto i = std::size_t(0); i != waiters.size(); ++i) {
      waiters[i] = spawn(consume, Details::Scheduler::DEFAULT_STACK_SIZE,
        i % contexts, results[i].get_eval());
    }
    for(auto i = std::size_t(0); i != threads.size(); ++i) {
      threads[i] = std::thread([&, i] {
        ready[i].get_eval().set();
        native_results[i] = consume();
      });
    }
    for(auto& result : ready) {
      result.get();
    }
    auto witnesses = std::array<Routine::Id, 2>();
    auto observed = std::array<Async<void>, 2>();
    for(auto i = std::size_t(0); i != witnesses.size(); ++i) {
      witnesses[i] = spawn([] {}, Details::Scheduler::DEFAULT_STACK_SIZE,
        i % contexts, observed[i].get_eval());
    }
    for(auto& result : observed) {
      result.get();
    }
    release.get_eval().set();
    for(auto& thread : threads) {
      thread.join();
    }
    for(auto& result : results) {
      REQUIRE(result.get() == 42);
    }
    for(auto result : native_results) {
      REQUIRE(result == 42);
    }
    auto is_exception = owner_result.get();
    wait(owner);
    for(auto waiter : waiters) {
      wait(waiter);
    }
    for(auto witness : witnesses) {
      wait(witness);
    }
    REQUIRE(is_exception == should_throw);
    REQUIRE(count == 1 + static_cast<int>(should_throw));
  }
}
