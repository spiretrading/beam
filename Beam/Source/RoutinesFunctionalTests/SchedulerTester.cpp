#include <doctest/doctest.h>
#include "Beam/Routines/Scheduler.hpp"

using namespace Beam;

TEST_SUITE("Scheduler") {
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
