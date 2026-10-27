#include <array>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Beam/Routines/Async.hpp"

using namespace Beam;

namespace {
  class ThrowingCopy {
    public:
      ThrowingCopy() = default;

      ThrowingCopy(const ThrowingCopy&) {
        throw std::runtime_error("copy failed");
      }
  };
}

TEST_SUITE("Async") {
  TEST_CASE("spawn_const_callable") {
    auto function = [value = 0] () mutable {
      return ++value;
    };
    auto result = Async<int>();
    auto stack_size = Details::Scheduler::DEFAULT_STACK_SIZE;
    auto id = Routine::Id();
    SUBCASE("spawn") {
      id = spawn(std::as_const(function), result.get_eval());
    }
    SUBCASE("stack_size") {
      id = spawn(std::as_const(function), stack_size, result.get_eval());
    }
    SUBCASE("context") {
      id = spawn(std::as_const(function), stack_size, 0, result.get_eval());
    }
    REQUIRE(result.get() == 1);
    wait(id);
    REQUIRE(function() == 1);
  }

  TEST_CASE("spawn_result_copy_exception") {
    auto value = ThrowingCopy();
    auto async = Async<ThrowingCopy>();
    auto eval = async.get_eval();
    auto results = std::array<Async<void>, 2>();
    auto consumers = std::array<Routine::Id, 2>();
    for(auto i = std::size_t(0); i != consumers.size(); ++i) {
      consumers[i] = spawn([&] {
        async.get();
      }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, results[i].get_eval());
    }
    auto producer = spawn([&] () -> const ThrowingCopy& {
      return value;
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, std::move(eval));
    for(auto& result : results) {
      REQUIRE_THROWS_AS(result.get(), std::runtime_error);
    }
    wait(producer);
    for(auto consumer : consumers) {
      wait(consumer);
    }
  }
}
