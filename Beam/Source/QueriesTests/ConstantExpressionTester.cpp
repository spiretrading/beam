#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ConstantExpression") {
  TEST_CASE("construct_from_int") {
    auto expression = ConstantExpression(42);
    REQUIRE(expression.get_type() == typeid(int));
    REQUIRE(expression.get_value().as<int>() == 42);
  }

  TEST_CASE("construct_from_string") {
    auto expression = ConstantExpression("hello");
    REQUIRE(expression.get_type() == typeid(std::string));
    REQUIRE(expression.get_value() == "hello");
  }

  TEST_CASE("copy") {
    auto original = ConstantExpression("world");
    auto copy = original;
    REQUIRE(copy.get_type() == typeid(std::string));
    REQUIRE(copy.get_value() == "world");
    REQUIRE(original.get_value() == "world");
  }

  TEST_CASE("stream") {
    auto expression = ConstantExpression(123);
    REQUIRE(to_string(expression) == "123");
  }

  TEST_CASE("bad_cast") {
    auto expression = ConstantExpression(100);
    REQUIRE_THROWS_AS(expression.get_value().as<double>(), std::bad_cast);
  }
}
