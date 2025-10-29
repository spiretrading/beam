#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Utilities/Expect.hpp"

using namespace Beam;

TEST_SUITE("Expect") {
  TEST_CASE("default_construction") {
    auto expect = Expect<int>();
    REQUIRE(expect.is_value());
    REQUIRE_FALSE(expect.is_exception());
  }

  TEST_CASE("value_construction") {
    auto expect = Expect(42);
    REQUIRE(expect.is_value());
    REQUIRE(expect.get() == 42);
  }

  TEST_CASE("move_value_construction") {
    auto value = std::string("test");
    auto expect = Expect(std::move(value));
    REQUIRE(expect.is_value());
    REQUIRE(expect.get() == "test");
  }

  TEST_CASE("exception_construction") {
    auto exception = std::make_exception_ptr(std::runtime_error("error"));
    auto expect = Expect<int>(exception);
    REQUIRE(expect.is_exception());
    REQUIRE_FALSE(expect.is_value());
  }

  TEST_CASE("get_throws_exception") {
    auto exception = std::make_exception_ptr(std::runtime_error("error"));
    auto expect = Expect<int>(exception);
    REQUIRE_THROWS_AS(expect.get(), std::runtime_error);
  }

  TEST_CASE("get_returns_lvalue_reference") {
    auto expect = Expect(42);
    auto& value = expect.get();
    value = 100;
    REQUIRE(expect.get() == 100);
  }

  TEST_CASE("get_returns_rvalue_reference") {
    auto expect = Expect<std::string>("test");
    auto value = std::move(expect).get();
    REQUIRE(value == "test");
  }

  TEST_CASE("implicit_conversion") {
    auto expect = Expect(42);
    auto value = static_cast<const int&>(expect);
    REQUIRE(value == 42);
  }

  TEST_CASE("get_exception_returns_exception") {
    auto exception = std::make_exception_ptr(std::runtime_error("error"));
    auto expect = Expect<int>(exception);
    REQUIRE(expect.get_exception() == exception);
  }

  TEST_CASE("get_exception_returns_null_for_value") {
    auto expect = Expect(42);
    REQUIRE(!expect.get_exception());
  }

  TEST_CASE("try_call_stores_value") {
    auto expect = Expect<int>();
    expect.try_call([] { return 42; });
    REQUIRE(expect.is_value());
    REQUIRE(expect.get() == 42);
  }

  TEST_CASE("try_call_stores_exception") {
    auto expect = Expect<int>();
    expect.try_call([] () -> int {
      throw std::runtime_error("error");
    });
    REQUIRE(expect.is_exception());
    REQUIRE_THROWS_AS(expect.get(), std::runtime_error);
  }

  TEST_CASE("assign_from_expect") {
    auto source = Expect(42);
    auto destination = Expect<int>();
    destination = source;
    REQUIRE(destination.get() == 42);
  }

  TEST_CASE("assign_from_expect_rvalue") {
    auto source = Expect<std::string>("test");
    auto destination = Expect<std::string>();
    destination = std::move(source);
    REQUIRE(destination.get() == "test");
  }

  TEST_CASE("assign_from_value") {
    auto expect = Expect<int>();
    expect = 42;
    REQUIRE(expect.get() == 42);
  }

  TEST_CASE("assign_from_value_rvalue") {
    auto expect = Expect<std::string>();
    expect = std::string("test");
    REQUIRE(expect.get() == "test");
  }

  TEST_CASE("void_default_construction") {
    auto expect = Expect<void>();
    REQUIRE(expect.is_value());
    REQUIRE_FALSE(expect.is_exception());
  }

  TEST_CASE("void_exception_construction") {
    auto exception = std::make_exception_ptr(std::runtime_error("error"));
    auto expect = Expect<void>(exception);
    REQUIRE(expect.is_exception());
  }

  TEST_CASE("void_get_does_not_throw") {
    auto expect = Expect<void>();
    REQUIRE_NOTHROW(expect.get());
  }

  TEST_CASE("void_get_throws_exception") {
    auto exception = std::make_exception_ptr(std::runtime_error("error"));
    auto expect = Expect<void>(exception);
    REQUIRE_THROWS_AS(expect.get(), std::runtime_error);
  }

  TEST_CASE("void_try_call_succeeds") {
    auto expect = Expect<void>();
    auto called = false;
    expect.try_call([&] { called = true; });
    REQUIRE(called);
    REQUIRE(expect.is_value());
  }

  TEST_CASE("void_try_call_catches_exception") {
    auto expect = Expect<void>();
    expect.try_call([] { throw std::runtime_error("error"); });
    REQUIRE(expect.is_exception());
  }

  TEST_CASE("try_call_function_returns_value") {
    auto result = try_call([] { return 42; });
    REQUIRE(result.is_value());
    REQUIRE(result.get() == 42);
  }

  TEST_CASE("try_call_function_catches_exception") {
    auto result = try_call([] () -> int {
      throw std::runtime_error("error");
    });
    REQUIRE(result.is_exception());
  }

  TEST_CASE("try_call_function_void_return") {
    auto called = false;
    auto result = try_call([&] { called = true; });
    REQUIRE(called);
    REQUIRE(result.is_value());
  }

  TEST_CASE("try_call_function_void_catches_exception") {
    auto result = try_call([] { throw std::runtime_error("error"); });
    REQUIRE(result.is_exception());
  }

  TEST_CASE("try_or_nest_returns_value") {
    auto result = try_or_nest([] { return 42; }, std::runtime_error("outer"));
    REQUIRE(result == 42);
  }

  TEST_CASE("try_or_nest_throws_nested_exception") {
    REQUIRE_THROWS_AS(
      try_or_nest([] () -> int {
        throw std::logic_error("inner");
      }, std::runtime_error("outer")),
      std::runtime_error);
  }

  TEST_CASE("nest_current_exception_creates_nested") {
    std::exception_ptr nested;
    try {
      throw std::logic_error("inner");
    } catch(...) {
      nested = nest_current_exception(std::runtime_error("outer"));
    }
    REQUIRE(nested);
    REQUIRE_THROWS_AS(std::rethrow_exception(nested), std::runtime_error);
  }
}
