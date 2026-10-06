#include <memory>
#include <string>
#include <boost/variant/variant.hpp>
#include <doctest/doctest.h>
#include "Beam/Utilities/OverloadSet.hpp"

using namespace Beam;
using namespace boost;

namespace {
  using BoostVariant = boost::variant<std::string>;
  using StandardVariant = std::variant<std::string>;
}

TEST_SUITE("OverloadSet") {
  TEST_CASE_TEMPLATE("named_callable", V, BoostVariant, StandardVariant) {
    auto value = V(std::string("hello"));
    auto callable = [] (const std::string& value) { return value.size(); };
    auto visitor = OverloadSet(callable);
    REQUIRE(visitor(std::string("hello")) == 5);
    REQUIRE(visit(value, callable) == 5);
  }

  TEST_CASE_TEMPLATE("move_only_callable", V, BoostVariant, StandardVariant) {
    auto value = V(std::string("hello"));
    auto callable = [offset = std::make_unique<int>(3)] (
        const std::string& value) {
      return value.size() + *offset;
    };
    REQUIRE(visit(value, std::move(callable)) == 8);
  }

  TEST_CASE_TEMPLATE("variant_forwarding", V, BoostVariant, StandardVariant) {
    auto value = V(std::string("hello"));
    auto& reference = visit(value,
      [] (std::string& value) -> std::string& { return value; });
    reference = "updated";
    auto& constant = visit(std::as_const(value),
      [] (const std::string& value) -> const std::string& { return value; });
    REQUIRE(&constant == &reference);
    REQUIRE(constant == "updated");
    auto&& moved = visit(std::move(value),
      [] (std::string&& value) -> std::string&& { return std::move(value); });
    REQUIRE(&moved == &reference);
    auto result = std::move(moved);
    REQUIRE(result == "updated");
  }

  TEST_CASE("single_lambda_with_int") {
    auto v = variant<int, double>(42);
    auto result = visit(v,
      [] (int value) { return value * 2; },
      [] (double value) { return static_cast<int>(value * 2); });
    REQUIRE(result == 84);
  }

  TEST_CASE("single_lambda_with_double") {
    auto v = variant<int, double>(3.14);
    auto result = visit(v,
      [] (int value) { return value * 2; },
      [] (double value) { return static_cast<int>(value * 2); });
    REQUIRE(result == 6);
  }

  TEST_CASE("multiple_types_with_string") {
    auto v = variant<int, double, std::string>("hello");
    auto result = visit(v,
      [] (int value) { return std::to_string(value); },
      [] (double value) { return std::to_string(value); },
      [] (const std::string& value) { return value + " world"; });
    REQUIRE(result == "hello world");
  }

  TEST_CASE("void_return_type") {
    auto v = variant<int, std::string>(123);
    auto called = false;
    auto value = 0;
    visit(v,
      [&] (int v) { called = true; value = v; },
      [&] (const std::string&) { called = true; });
    REQUIRE(called);
    REQUIRE(value == 123);
  }

  TEST_CASE("make_overload_set_creates_visitor") {
    auto visitor = make_overload_set(
      [] (int value) { return value * 3; },
      [] (double value) { return static_cast<int>(value * 3); });
    auto v1 = variant<int, double>(10);
    auto v2 = variant<int, double>(2.5);
    auto result1 = apply_visitor(visitor, v1);
    auto result2 = apply_visitor(visitor, v2);
    REQUIRE(result1 == 30);
    REQUIRE(result2 == 7);
  }

  TEST_CASE("visitor_with_const_reference") {
    auto v = variant<std::string, int>("test");
    auto result = visit(v,
      [] (const std::string& str) { return str.length(); },
      [] (int value) { return static_cast<std::size_t>(value); });
    REQUIRE(result == 4);
  }

  TEST_CASE("visitor_with_forwarding") {
    auto visitor = make_overload_set(
      [] (int&& value) { return value + 1; },
      [] (double&& value) { return static_cast<int>(value + 1); });
    auto v = variant<int, double>(10);
    auto result = apply_visitor(visitor, v);
    REQUIRE(result == 11);
  }

  TEST_CASE("nested_variant_visitation") {
    using InnerVariant = variant<int, double>;
    using OuterVariant = variant<InnerVariant, std::string>;
    auto inner = InnerVariant(42);
    auto outer = OuterVariant(inner);
    auto result = visit(outer,
      [] (const InnerVariant& inner_variant) {
        return visit(inner_variant,
          [] (int value) { return value * 2; },
          [] (double value) { return static_cast<int>(value * 2); });
      },
      [] (const std::string& str) { return static_cast<int>(str.length()); });
    REQUIRE(result == 84);
  }

  TEST_CASE("visitor_preserves_value_category") {
    auto v = variant<int, std::string>(100);
    auto result = 0;
    visit(v,
      [&] (int value) { result = value; },
      [&] (const std::string& str) { result = str.length(); });
    REQUIRE(result == 100);
  }

  TEST_CASE("multiple_calls_to_same_visitor") {
    auto visitor = make_overload_set(
      [] (int value) { return value + 10; },
      [] (double value) { return static_cast<int>(value + 10); });
    auto v1 = variant<int, double>(5);
    auto v2 = variant<int, double>(7.5);
    auto result1 = apply_visitor(visitor, v1);
    auto result2 = apply_visitor(visitor, v2);
    REQUIRE(result1 == 15);
    REQUIRE(result2 == 17);
  }

  TEST_CASE("empty_variant_handling") {
    auto v = variant<int, double>();
    auto result = visit(v,
      [] (int value) { return value; },
      [] (double value) { return static_cast<int>(value); });
    REQUIRE(result == 0);
  }

  TEST_CASE("visitor_with_exception_handling") {
    auto v = variant<int, std::string>("test");
    auto did_throw = false;
    try {
      visit(v,
        [] (int) { throw std::runtime_error("int error"); },
        [] (const std::string&) { throw std::runtime_error("string error"); });
    } catch(const std::runtime_error& e) {
      did_throw = true;
      REQUIRE(std::string(e.what()) == "string error");
    }
    REQUIRE(did_throw);
  }
}
