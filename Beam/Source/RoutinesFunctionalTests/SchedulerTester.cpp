#include <latch>
#include <doctest/doctest.h>
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

TEST_SUITE("Scheduler") {
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
