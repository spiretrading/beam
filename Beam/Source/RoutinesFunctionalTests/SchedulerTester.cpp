#include <array>
#include <cstdint>
#include <latch>
#include <stdexcept>
#include <doctest/doctest.h>
#ifdef WIN32
  #include <windows.h>
#endif
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace Beam::Tests {
  struct SchedulerTester {
    static auto& get_idle_mutex(Details::Scheduler& scheduler) {
      return scheduler.m_idle_mutex;
    }

    static auto& get_idle_evals(Details::Scheduler& scheduler) {
      return scheduler.m_idle_evals;
    }
  };
}

namespace {
  class ThrowingCopy {
    public:
      explicit ThrowingCopy(bool& is_called) noexcept
        : m_is_called(&is_called) {}

      ThrowingCopy(const ThrowingCopy&) {
        throw std::runtime_error("copy failed");
      }

      ThrowingCopy(ThrowingCopy&&) noexcept = default;

      void operator ()() const {
        *m_is_called = true;
      }

    private:
      bool* m_is_called;
  };

  struct StackBounds {
    std::uintptr_t m_base;
    std::size_t m_extent;
    std::uintptr_t m_address;
#ifdef WIN32
    std::uintptr_t m_native_base;
    std::size_t m_native_extent;
#endif
  };

  StackBounds get_stack_bounds() {
    auto& routine = static_cast<ScheduledRoutine&>(get_current_routine());
    auto bounds = StackBounds();
    bounds.m_base = reinterpret_cast<std::uintptr_t>(routine.get_stack_base());
    bounds.m_extent = routine.get_stack_extent();
    bounds.m_address = reinterpret_cast<std::uintptr_t>(&bounds);
#ifdef WIN32
    auto base = ULONG_PTR();
    auto end = ULONG_PTR();
    GetCurrentThreadStackLimits(&base, &end);
    bounds.m_native_base = base;
    bounds.m_native_extent = end - base;
#endif
    return bounds;
  }
}

TEST_SUITE("Scheduler") {
  TEST_CASE("spawn_copy_exception") {
    auto scheduler = Details::Scheduler();
    auto is_called = false;
    auto function = ThrowingCopy(is_called);
    auto stack_size = Details::Scheduler::DEFAULT_STACK_SIZE;
    REQUIRE(!noexcept(FunctionRoutine(function, stack_size, 0)));
    REQUIRE_THROWS_AS(
      scheduler.spawn(function, stack_size, 0), std::runtime_error);
    REQUIRE(!is_called);
    REQUIRE(scheduler.is_idle());
    REQUIRE(noexcept(FunctionRoutine(std::move(function), stack_size, 0)));
    scheduler.spawn(std::move(function), stack_size, 0);
    scheduler.stop();
    REQUIRE(is_called);
  }

  TEST_CASE("idle_notification_after_new_work") {
    auto scheduler = Details::Scheduler();
    auto idle = Async<void>();
    scheduler.notify_when_idle(idle.get_eval());
    idle.get();
    auto lock = std::unique_lock(SchedulerTester::get_idle_mutex(scheduler));
    scheduler.spawn([] {}, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    while(!scheduler.is_idle()) {
      std::this_thread::yield();
    }
    auto started = std::latch(1);
    auto release = std::latch(1);
    scheduler.spawn([&] {
      started.count_down();
      release.wait();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    SchedulerTester::get_idle_evals(scheduler).push_back(idle.get_eval());
    lock.unlock();
    started.wait();
    lock.lock();
    auto is_pending = !SchedulerTester::get_idle_evals(scheduler).empty();
    lock.unlock();
    release.count_down();
    idle.get();
    scheduler.stop();
    REQUIRE(is_pending);
    REQUIRE(scheduler.is_idle());
  }

  TEST_CASE("stack_bounds") {
    auto stack_size = Details::Scheduler::DEFAULT_STACK_SIZE;
    SUBCASE("default_size") {}
    SUBCASE("custom_size") {
      stack_size *= 2;
    }
    auto ready = Async<void>();
    auto release = Async<void>();
    auto result = Async<std::array<StackBounds, 3>>();
    auto routine = spawn([&] {
      auto bounds = std::array<StackBounds, 3>();
      bounds[0] = get_stack_bounds();
      defer();
      bounds[1] = get_stack_bounds();
      ready.get_eval().set();
      release.get();
      bounds[2] = get_stack_bounds();
      return bounds;
    }, stack_size, 0, result.get_eval());
    ready.get();
    auto notifier = spawn([&] {
      release.get_eval().set();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto bounds = result.get();
    wait(routine);
    wait(notifier);
    for(auto& bound : bounds) {
      REQUIRE(bound.m_extent > 0);
      REQUIRE(bound.m_address >= bound.m_base);
      REQUIRE(bound.m_address - bound.m_base < bound.m_extent);
#ifdef WIN32
      REQUIRE(bound.m_base == bound.m_native_base);
      REQUIRE(bound.m_extent == bound.m_native_extent);
#endif
    }
  }

  TEST_CASE("stop") {
    auto scheduler = Details::Scheduler();
    REQUIRE(scheduler.get_thread_count() > 0);
    scheduler.stop();
    scheduler.stop();
  }

  TEST_CASE("default_context") {
    auto& scheduler = Details::Scheduler::get();
    auto result = Async<std::size_t>();
    auto routine = spawn([&] {
      return static_cast<ScheduledRoutine&>(get_current_routine()).
        get_context_id();
    }, result.get_eval());
    REQUIRE(result.get() == routine % scheduler.get_thread_count());
    wait(routine);
  }
}
