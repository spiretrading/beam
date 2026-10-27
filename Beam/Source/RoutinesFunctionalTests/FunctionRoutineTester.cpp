#include <stdexcept>
#include <doctest/doctest.h>
#include "Beam/Routines/FunctionRoutine.hpp"

using namespace Beam;

namespace {
  auto calls = 0;

  void count_call() {
    ++calls;
  }

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
}

TEST_SUITE("FunctionRoutine") {
  TEST_CASE("free_function") {
    calls = 0;
    auto stack_size = Details::Scheduler::DEFAULT_STACK_SIZE;
    SUBCASE("construction") {
      auto routine = FunctionRoutine(count_call, stack_size, 0);
      routine.advance();
    }
    SUBCASE("spawn") {
      wait(spawn(count_call));
    }
    SUBCASE("stack_size") {
      wait(spawn(count_call, stack_size));
    }
    SUBCASE("context") {
      wait(spawn(count_call, stack_size, 0));
    }
    REQUIRE(calls == 1);
  }

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
}
