#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <doctest/doctest.h>
#include "Beam/Utilities/SharedCallable.hpp"

using namespace Beam;

namespace {
  struct MoveOnlyCallable {
    int m_value;

    MoveOnlyCallable(int value)
      : m_value(value) {}

    MoveOnlyCallable(const MoveOnlyCallable&) = delete;

    MoveOnlyCallable(MoveOnlyCallable&& other) noexcept
        : m_value(other.m_value) {
      other.m_value = 0;
    }

    MoveOnlyCallable& operator =(const MoveOnlyCallable&) = delete;
    MoveOnlyCallable& operator =(MoveOnlyCallable&&) = delete;

    int operator ()() const {
      return m_value;
    }

    int operator ()(int increment) const {
      return m_value + increment;
    }
  };

  struct ThrowingCallable {
    void operator ()() const {
      throw std::runtime_error("error");
    }
  };

  struct NoexceptCallable {
    int operator ()() const noexcept {
      return 42;
    }
  };
}

TEST_SUITE("SharedCallable") {
  TEST_CASE("construct_from_move_only_callable") {
    auto callable = SharedCallable(MoveOnlyCallable(123));
    REQUIRE(callable() == 123);
  }

  TEST_CASE("construct_from_lambda") {
    auto value = 456;
    auto lambda = [value] { return value; };
    auto callable = SharedCallable(lambda);
    REQUIRE(callable() == 456);
  }

  TEST_CASE("construct_from_mutable_lambda") {
    auto counter = 0;
    auto lambda = [counter] () mutable { return ++counter; };
    auto callable = SharedCallable(lambda);
    REQUIRE(callable() == 1);
    REQUIRE(callable() == 2);
    REQUIRE(callable() == 3);
  }

  TEST_CASE("copy_shared_callable") {
    auto callable1 = SharedCallable(MoveOnlyCallable(789));
    auto callable2 = callable1;
    REQUIRE(callable1() == 789);
    REQUIRE(callable2() == 789);
  }

  TEST_CASE("invoke_with_arguments") {
    auto callable = SharedCallable(MoveOnlyCallable(100));
    REQUIRE(callable(23) == 123);
    REQUIRE(callable(50) == 150);
  }

  TEST_CASE("invoke_const_callable") {
    auto const callable = SharedCallable(MoveOnlyCallable(200));
    REQUIRE(callable() == 200);
  }

  TEST_CASE("get_callable") {
    auto callable = SharedCallable(MoveOnlyCallable(300));
    auto& wrapped = callable.get_callable();
    REQUIRE(wrapped() == 300);
  }

  TEST_CASE("get_callable_const") {
    auto const callable = SharedCallable(MoveOnlyCallable(400));
    auto& wrapped = callable.get_callable();
    REQUIRE(wrapped() == 400);
  }

  TEST_CASE("multiple_invocations_share_state") {
    auto counter = std::make_shared<int>(0);
    auto lambda = [counter] { return ++(*counter); };
    auto callable = SharedCallable(lambda);
    REQUIRE(callable() == 1);
    REQUIRE(callable() == 2);
    auto copy = callable;
    REQUIRE(copy() == 3);
    REQUIRE(callable() == 4);
  }

  TEST_CASE("noexcept_callable") {
    auto callable = SharedCallable(NoexceptCallable());
    REQUIRE(noexcept(callable()));
    REQUIRE(callable() == 42);
  }

  TEST_CASE("throwing_callable") {
    auto callable = SharedCallable(ThrowingCallable());
    REQUIRE_FALSE(noexcept(callable()));
    REQUIRE_THROWS_AS(callable(), std::runtime_error);
  }

  TEST_CASE("callable_returns_reference") {
    auto value = 100;
    auto lambda = [&value] () -> int& { return value; };
    auto callable = SharedCallable(lambda);
    callable() = 200;
    REQUIRE(value == 200);
  }

  TEST_CASE("callable_with_multiple_parameters") {
    auto lambda = [] (int a, int b, int c) { return a + b + c; };
    auto callable = SharedCallable(lambda);
    REQUIRE(callable(1, 2, 3) == 6);
    REQUIRE(callable(10, 20, 30) == 60);
  }

  TEST_CASE("callable_with_move_only_parameters") {
    auto lambda = [] (std::unique_ptr<int> pointer) { return *pointer; };
    auto callable = SharedCallable(lambda);
    REQUIRE(callable(std::make_unique<int>(999)) == 999);
  }
}
