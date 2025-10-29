#include <string>
#include <boost/variant/variant.hpp>
#include <doctest/doctest.h>
#include "Beam/Utilities/VariantLambdaVisitor.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("VariantLambdaVisitor") {
  TEST_CASE("single_lambda_with_int") {
    auto v = variant<int, double>(42);
    auto result = apply_variant_lambda_visitor(v,
      [] (int value) { return value * 2; },
      [] (double value) { return static_cast<int>(value * 2); });
    REQUIRE(result == 84);
  }

  TEST_CASE("single_lambda_with_double") {
    auto v = variant<int, double>(3.14);
    auto result = apply_variant_lambda_visitor(v,
      [] (int value) { return value * 2; },
      [] (double value) { return static_cast<int>(value * 2); });
    REQUIRE(result == 6);
  }

  TEST_CASE("multiple_types_with_string") {
    auto v = variant<int, double, std::string>("hello");
    auto result = apply_variant_lambda_visitor(v,
      [] (int value) { return std::to_string(value); },
      [] (double value) { return std::to_string(value); },
      [] (const std::string& value) { return value + " world"; });
    REQUIRE(result == "hello world");
  }

  TEST_CASE("void_return_type") {
    auto v = variant<int, std::string>(123);
    auto called = false;
    auto value = 0;
    apply_variant_lambda_visitor(v,
      [&] (int v) { called = true; value = v; },
      [&] (const std::string&) { called = true; });
    REQUIRE(called);
    REQUIRE(value == 123);
  }

  TEST_CASE("make_variant_lambda_visitor_creates_visitor") {
    auto visitor = make_variant_lambda_visitor(
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
    auto result = apply_variant_lambda_visitor(v,
      [] (const std::string& str) { return str.length(); },
      [] (int value) { return static_cast<std::size_t>(value); });
    REQUIRE(result == 4);
  }

  TEST_CASE("visitor_with_forwarding") {
    auto visitor = make_variant_lambda_visitor(
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
    auto result = apply_variant_lambda_visitor(outer,
      [] (const InnerVariant& inner_variant) {
        return apply_variant_lambda_visitor(inner_variant,
          [] (int value) { return value * 2; },
          [] (double value) { return static_cast<int>(value * 2); });
      },
      [] (const std::string& str) { return static_cast<int>(str.length()); });
    REQUIRE(result == 84);
  }

  TEST_CASE("visitor_preserves_value_category") {
    auto v = variant<int, std::string>(100);
    auto result = 0;
    apply_variant_lambda_visitor(v,
      [&] (int value) { result = value; },
      [&] (const std::string& str) { result = str.length(); });
    REQUIRE(result == 100);
  }

  TEST_CASE("multiple_calls_to_same_visitor") {
    auto visitor = make_variant_lambda_visitor(
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
    auto result = apply_variant_lambda_visitor(v,
      [] (int value) { return value; },
      [] (double value) { return static_cast<int>(value); });
    REQUIRE(result == 0);
  }

  TEST_CASE("visitor_with_exception_handling") {
    auto v = variant<int, std::string>("test");
    auto did_throw = false;
    try {
      apply_variant_lambda_visitor(v,
        [] (int) { throw std::runtime_error("int error"); },
        [] (const std::string&) { throw std::runtime_error("string error"); });
    } catch(const std::runtime_error& e) {
      did_throw = true;
      REQUIRE(std::string(e.what()) == "string error");
    }
    REQUIRE(did_throw);
  }
}
