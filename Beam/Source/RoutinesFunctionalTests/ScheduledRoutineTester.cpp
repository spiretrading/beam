#include <array>
#include <cstdint>
#include <doctest/doctest.h>
#if defined(_MSC_VER) && defined(_DEBUG)
  #include <crtdbg.h>
#endif
#ifdef WIN32
  #include <windows.h>
#endif
#include "Beam/Routines/ScheduledRoutine.hpp"

using namespace Beam;

namespace {
  struct StackBounds {
    std::uintptr_t m_base;
    std::size_t m_extent;
    std::uintptr_t m_address;
#ifdef WIN32
    std::uintptr_t m_native_base;
    std::size_t m_native_extent;
    std::size_t m_committed_extent;
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
    auto memory = MEMORY_BASIC_INFORMATION();
    VirtualQuery(&bounds, &memory, sizeof(memory));
    auto allocation = memory.AllocationBase;
    auto address = reinterpret_cast<std::uintptr_t>(allocation);
    while(VirtualQuery(reinterpret_cast<void*>(address), &memory,
        sizeof(memory)) != 0 && memory.AllocationBase == allocation) {
      if(memory.State == MEM_COMMIT) {
        bounds.m_committed_extent += memory.RegionSize;
      }
      address += memory.RegionSize;
    }
#endif
    return bounds;
  }

#if defined(_MSC_VER) && defined(_DEBUG)
  thread_local auto maximum_allocation = std::size_t(0);

  int record_allocation(int type, void*, std::size_t size, int, long,
      const unsigned char*, int) {
    if(type == _HOOK_ALLOC) {
      maximum_allocation = std::max(maximum_allocation, size);
    }
    return true;
  }

  class AllocationMonitor {
    public:
      AllocationMonitor() noexcept;
      ~AllocationMonitor();

    private:
      _CRT_ALLOC_HOOK m_previous;
  };

  AllocationMonitor::AllocationMonitor() noexcept
      : m_previous(_CrtSetAllocHook(&record_allocation)) {
    maximum_allocation = 0;
  }

  AllocationMonitor::~AllocationMonitor() {
    _CrtSetAllocHook(m_previous);
  }
#endif
}

TEST_SUITE("ScheduledRoutine") {
#if defined(_MSC_VER) && defined(_DEBUG)
  TEST_CASE("control_storage") {
    auto stack_size = Details::Scheduler::DEFAULT_STACK_SIZE;
    SUBCASE("default_size") {}
    SUBCASE("custom_size") {
      stack_size *= 2;
    }
    auto routine = FunctionRoutine([] {
      defer();
    }, stack_size, 0);
    {
      auto monitor = AllocationMonitor();
      routine.advance();
    }
    routine.advance();
    REQUIRE(maximum_allocation < 4096);
  }
#endif

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
      REQUIRE(bound.m_committed_extent >= stack_size);
#endif
    }
  }
}
