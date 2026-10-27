#include <array>
#include <atomic>
#include <cstdlib>
#include <latch>
#include <memory>
#include <new>
#include <vector>
#include <boost/config.hpp>
#include <doctest/doctest.h>
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  thread_local auto allocations_until_failure = -1;

  class AllocationFailureFunction {
    public:
      AllocationFailureFunction(
        std::shared_ptr<int> value, int allocations) noexcept;
      AllocationFailureFunction(
        const AllocationFailureFunction& function) noexcept;

      void operator ()() const;

    private:
      std::shared_ptr<int> m_value;
      int m_allocations;
  };

  AllocationFailureFunction::AllocationFailureFunction(
    std::shared_ptr<int> value, int allocations) noexcept
    : m_value(std::move(value)),
      m_allocations(allocations) {}

  AllocationFailureFunction::AllocationFailureFunction(
      const AllocationFailureFunction& function) noexcept
      : m_value(function.m_value),
        m_allocations(function.m_allocations) {
    allocations_until_failure = m_allocations;
  }

  void AllocationFailureFunction::operator ()() const {
    ++*m_value;
  }
}

BOOST_NOINLINE void* operator new(std::size_t size) {
  if(allocations_until_failure == 0) {
    allocations_until_failure = -1;
    throw std::bad_alloc();
  }
  if(allocations_until_failure > 0) {
    --allocations_until_failure;
  }
  if(auto memory = std::malloc(std::max(size, std::size_t(1)))) {
    return memory;
  }
  throw std::bad_alloc();
}

BOOST_NOINLINE void operator delete(void* memory) noexcept {
  std::free(memory);
}

BOOST_NOINLINE void operator delete(void* memory, std::size_t) noexcept {
  std::free(memory);
}

namespace Beam::Tests {
  struct SchedulerTester {
    static auto& get_routine_ids(Details::Scheduler& scheduler) {
      return scheduler.m_routine_ids;
    }

    static auto& get_idle_mutex(Details::Scheduler& scheduler) {
      return scheduler.m_idle_mutex;
    }

    static auto& get_idle_evals(Details::Scheduler& scheduler) {
      return scheduler.m_idle_evals;
    }
  };
}

TEST_SUITE("Scheduler") {
  TEST_CASE("spawn_allocation_failure") {
    auto allocations = 0;
    SUBCASE("registration") {}
    SUBCASE("queue") {
      allocations = 1;
    }
    auto scheduler = Details::Scheduler();
    auto limit = std::size_t(1024);
    with(SchedulerTester::get_routine_ids(scheduler), [&] (auto& ids) {
      ids.reserve(limit + 1);
    });
    auto started = std::latch(1);
    auto release = std::latch(1);
    scheduler.spawn([&] {
      started.count_down();
      release.wait();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    started.wait();
    auto failed = false;
    auto resource = std::weak_ptr<int>();
    for(auto i = std::size_t(0); i != limit && !failed; ++i) {
      auto value = std::make_shared<int>();
      resource = value;
      auto function = AllocationFailureFunction(value, allocations);
      try {
        scheduler.spawn(function, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
      } catch(const std::bad_alloc&) {
        failed = true;
      }
      allocations_until_failure = -1;
    }
    release.count_down();
    scheduler.stop();
    auto routine_count = with(
      SchedulerTester::get_routine_ids(scheduler), [] (const auto& ids) {
        return ids.size();
      });
    CAPTURE(scheduler.get_busy_count());
    CAPTURE(routine_count);
    REQUIRE(failed);
    REQUIRE(resource.expired());
    REQUIRE(scheduler.is_idle());
    REQUIRE(routine_count == 0);
  }

  TEST_CASE("suspend_resume") {
    auto& scheduler = Details::Scheduler::get();
    auto routines = std::array<Routine*, 3>();
    auto ids = std::vector<Routine::Id>();
    auto completed = std::atomic_int();
    for(auto& routine : routines) {
      ids.push_back(spawn([&] {
        routine = &get_current_routine();
        suspend();
        suspend();
        ++completed;
      }, Details::Scheduler::DEFAULT_STACK_SIZE, 0));
    }
    auto busy_counts = std::array<std::size_t, 2>();
    for(auto& count : busy_counts) {
      auto idle = Async<void>();
      scheduler.notify_when_idle(idle.get_eval());
      idle.get();
      count = scheduler.get_busy_count();
      for(auto i = routines.rbegin(); i != routines.rend(); ++i) {
        (*i)->resume();
      }
    }
    for(auto id : ids) {
      wait(id);
    }
    REQUIRE(completed == routines.size());
    for(auto count : busy_counts) {
      REQUIRE(count == 0);
    }
  }

  TEST_CASE("resume_pending_context") {
    auto routine = static_cast<Routine*>(nullptr);
    auto order = std::vector<int>();
    auto resumed = spawn([&] {
      routine = &get_current_routine();
      suspend();
      order.push_back(2);
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto idle = Async<void>();
    Details::Scheduler::get().notify_when_idle(idle.get_eval());
    idle.get();
    auto started = std::latch(1);
    auto release = std::latch(1);
    auto blocker = spawn([&] {
      started.count_down();
      release.wait();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    started.wait();
    auto pending = spawn([&] {
      order.push_back(1);
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    routine->resume();
    release.count_down();
    wait(blocker);
    wait(pending);
    wait(resumed);
    REQUIRE(order == std::vector({1, 2}));
  }

  TEST_CASE("early_resume") {
    auto routine = static_cast<Routine*>(nullptr);
    auto pending = std::latch(1);
    auto release = std::latch(1);
    auto completed = false;
    auto id = spawn([&] {
      routine = &get_current_routine();
      routine->pending_suspend();
      pending.count_down();
      release.wait();
      suspend();
      completed = true;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    pending.wait();
    routine->resume();
    release.count_down();
    wait(id);
    REQUIRE(completed);
  }

  TEST_CASE("defer_order") {
    auto order = std::vector<int>();
    auto started = std::latch(1);
    auto release = std::latch(1);
    auto first = spawn([&] {
      started.count_down();
      release.wait();
      order.push_back(1);
      defer();
      order.push_back(3);
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    started.wait();
    auto second = spawn([&] {
      order.push_back(2);
      defer();
      order.push_back(4);
      defer();
      order.push_back(5);
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    release.count_down();
    wait(first);
    wait(second);
    REQUIRE(order == std::vector({1, 2, 3, 4, 5}));
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
