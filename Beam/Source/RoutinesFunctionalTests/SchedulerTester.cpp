#include <array>
#include <cstdint>
#include <doctest/doctest.h>
#ifdef WIN32
  #include <windows.h>
#endif
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;

namespace {
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
