#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Beam/Routines/Async.hpp"

using namespace Beam;

TEST_SUITE("Async") {
  TEST_CASE("set_and_get_value") {
    auto async = Async<int>();
    auto eval = async.get_eval();
    auto setter = std::async(
      std::launch::async, [eval = std::move(eval)] () mutable {
        eval.set(42);
      });
    auto value = async.get();
    REQUIRE(value == 42);
    setter.wait();
  }

  TEST_CASE("exception_propagates_to_get") {
    auto async = Async<int>();
    auto eval = async.get_eval();
    auto setter = std::async(
      std::launch::async, [eval = std::move(eval)] () mutable {
        eval.set_exception(std::runtime_error("test-exception"));
      });
    REQUIRE_THROWS_AS(async.get(), std::runtime_error);
    setter.wait();
  }

  TEST_CASE("eval_is_empty_and_move_semantics") {
    auto async = Async<int>();
    auto eval1 = async.get_eval();
    REQUIRE(!eval1.is_empty());
    auto eval2 = std::move(eval1);
    REQUIRE(!eval2.is_empty());
    REQUIRE(eval1.is_empty());
    eval2.set(7);
    REQUIRE(async.get() == 7);
  }

  TEST_CASE("void_specialization_set_and_get") {
    auto async = Async<void>();
    auto eval = async.get_eval();
    auto setter = std::async(
      std::launch::async, [eval = std::move(eval)] () mutable {
        eval.set();
      });
    async.get();
    setter.wait();
  }

  TEST_CASE("reset_allows_reuse") {
    auto async = Async<int>();
    {
      auto eval = async.get_eval();
      eval.set(1);
      REQUIRE(async.get() == 1);
    }
    async.reset();
    {
      auto eval = async.get_eval();
      eval.set(2);
      REQUIRE(async.get() == 2);
    }
  }

  TEST_CASE("multiple_waiters_are_resumed") {
    auto async = Async<int>();
    auto eval = async.get_eval();
    auto waiter = [] (auto a) {
      return std::async(std::launch::async, [=] () {
        return a->get();
      });
    };
    auto f1 = waiter(&async);
    auto f2 = waiter(&async);
    auto f3 = waiter(&async);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    eval.set(123);
    REQUIRE(f1.get() == 123);
    REQUIRE(f2.get() == 123);
    REQUIRE(f3.get() == 123);
  }

  TEST_CASE("move_only_result_is_moved_through") {
    auto async = Async<std::unique_ptr<int>>();
    auto eval = async.get_eval();
    auto setter = std::async(
      std::launch::async, [eval = std::move(eval)] () mutable {
        eval.set(std::make_unique<int>(77));
      });
    auto ptr = std::move(async.get());
    REQUIRE(ptr);
    REQUIRE(*ptr == 77);
    setter.wait();
  }
}
