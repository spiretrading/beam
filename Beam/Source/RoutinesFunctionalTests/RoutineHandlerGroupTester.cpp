#include <array>
#include <atomic>
#include <latch>
#include <thread>
#include <doctest/doctest.h>
#include "Beam/Routines/RoutineHandlerGroup.hpp"

using namespace Beam;

namespace {
  template<typename F>
  concept IsSpawnable = requires(RoutineHandlerGroup group) {
    group.spawn(std::declval<F>());
  };
}

TEST_SUITE("RoutineHandlerGroup") {
  TEST_CASE("spawn_constraints") {
    auto function = [] {};
    REQUIRE(IsSpawnable<decltype(function)>);
    REQUIRE(!IsSpawnable<int>);
  }

  TEST_CASE("same_context_wait") {
    auto result = Async<int>();
    auto routine = spawn([&] {
      auto group = RoutineHandlerGroup();
      auto count = 0;
      group.add(spawn([&] {
        group.add(spawn([&] {
          ++count;
        }, Details::Scheduler::DEFAULT_STACK_SIZE, 0));
        ++count;
      }, Details::Scheduler::DEFAULT_STACK_SIZE, 0));
      group.wait();
      group.wait();
      return count;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    auto count = result.get();
    wait(routine);
    REQUIRE(count == 2);
  }

  TEST_CASE("concurrent_add") {
    Details::Scheduler::get();
    auto group = RoutineHandlerGroup();
    auto count = std::atomic_int();
    auto ready = std::latch(4);
    auto threads = std::array<std::thread, 4>();
    for(auto& thread : threads) {
      thread = std::thread([&] {
        ready.arrive_and_wait();
        for(auto i = 0; i != 32; ++i) {
          group.spawn([&] {
            ++count;
          });
        }
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    group.wait();
    REQUIRE(count == 128);
  }
}
