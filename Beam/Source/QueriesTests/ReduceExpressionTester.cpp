#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ReduceExpression.hpp"

using namespace Beam;

TEST_SUITE("ReduceExpression") {
  TEST_CASE("compatible_type_constructor") {
    auto int_reducer = ConstantExpression(1);
    auto int_series = ConstantExpression(2);
    auto initial_int = Value(3);
    auto reduce_int = ReduceExpression(int_reducer, int_series, initial_int);
    REQUIRE(reduce_int.get_type() == typeid(int));
    REQUIRE(reduce_int.get_reducer().as<ConstantExpression>().get_value().
      as<int>() == int_reducer.get_value().as<int>());
    REQUIRE(reduce_int.get_series().as<ConstantExpression>().get_value().
      as<int>() == int_series.get_value().as<int>());
    REQUIRE(reduce_int.get_initial_value() == initial_int);
    auto string_reducer = ConstantExpression("hello world");
    auto string_series = ConstantExpression("goodbye sky");
    auto initial_string = Value(std::string("foo bar"));
    auto reduce_string =
      ReduceExpression(string_reducer, string_series, initial_string);
    REQUIRE(reduce_string.get_type() == typeid(std::string));
    REQUIRE(reduce_string.get_reducer().as<ConstantExpression>().get_value().
      as<std::string>() == string_reducer.get_value().as<std::string>());
    REQUIRE(reduce_string.get_series().as<ConstantExpression>().get_value().
      as<std::string>() == string_series.get_value().as<std::string>());
    REQUIRE(reduce_string.get_initial_value() == initial_string);
  }

  TEST_CASE("incompatible_type_constructor") {
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(0),
      ConstantExpression(0), std::string()), TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(0),
      ConstantExpression(std::string()), 0),
      TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(std::string("")),
      ConstantExpression(0), 321), TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(0),
      ConstantExpression(std::string()), std::string("hello")),
      TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(std::string()),
      ConstantExpression(0), std::string("goodbye")),
      TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(std::string()),
      ConstantExpression(std::string()), 123), TypeCompatibilityException);
  }

  TEST_CASE("stream") {
    auto reducer = ConstantExpression(1);
    auto series = ConstantExpression(2);
    auto initial_value = Value(3);
    auto reduce = ReduceExpression(reducer, series, initial_value);
    auto ss = std::stringstream();
    ss << reduce;
    REQUIRE(ss.str() == "(reduce 1 2 3)");
  }
}
